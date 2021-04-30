/*
 * @file GPSDControl.cpp
 *
 *  Created on: Jul 21, 2020
 *      @author: Gregory M. Baumgardner
 */

#include "LocationPlugin.h"
#include "GPSDWorker.h"

#include <atomic>
#include <bitset>
#include <cctype>
#include <cerrno>
#include <cstdlib>
#include <mutex>
#include <regex>
#include <sys/select.h>
#include <sys/socket.h>
#include <vector>

#define USE_STD_CHRONO
#include <FrequencyThrottle.h>
#include <rtcm/RtcmEncodedMessage.h>
#include <rtcm/RtcmMessage.h>

#include <tmx/messages/TmxNmea.hpp>
#include <Clock.h>
#include <LocationMessageEnumTypes.h>
#include <System.h>
#include <Uuid.h>

using namespace std;
using namespace tmx;
using namespace tmx::messages;
using namespace tmx::messages::location;
using namespace tmx::messages::rtcm;
using namespace tmx::utils;

#define GPSD_GROUP 0xB8

namespace LocationPlugin {

static FrequencyThrottle<size_t, chrono::milliseconds> _sampleThrottle { chrono::milliseconds(5) };
static FrequencyThrottle<int, chrono::seconds> _statusThrottle { chrono::seconds(3) };

static std::atomic<bool> _detectRtcm2 { false };
static std::atomic<bool> _detectRtcm3 { false };
static std::atomic<bool> _detectUblox { false };
static std::atomic<bool> _detect3DFix { false };
static std::atomic<bool> _detectDGPS  { false };

static string baseCmd = "PATH=$PATH:/usr/bin:/usr/local/bin ubxtool ";


// Need a temporary class to correctly set missing data for RTCM messages
// coming from the GPSD

FixTypes get_fix(int fix) {
	switch (fix) {
	case MODE_NO_FIX:
		return FixTypes::NoFix;
	case MODE_2D:
		return FixTypes::TwoD;
	case MODE_3D:
		return FixTypes::ThreeD;
	default:
		return FixTypes::Unknown;
	}
}

SignalQualityTypes get_quality(int quality) {
	switch (quality) {
	case STATUS_FIX:
		return SignalQualityTypes::GPS;
	case STATUS_RTK_FLT:
		return SignalQualityTypes::FloatRTK;
	case STATUS_RTK_FIX:
		return SignalQualityTypes::RealTimeKinematic;
	case STATUS_DGPS_FIX:
		return SignalQualityTypes::DGPS;
	case STATUS_DR:
	case STATUS_GNSSDR:
		return SignalQualityTypes::DeadReckoning;
	case STATUS_TIME:
		return SignalQualityTypes::ManualInputMode;
	case STATUS_SIM:
		return SignalQualityTypes::SimulationMode;
	case STATUS_PPS_FIX:
		return SignalQualityTypes::PPS;
	default:
		return SignalQualityTypes::Invalid;
	}
}

void LocationPlugin::ConfigureDevice() {
	static vector<string> cfgOptions;

	string cmd;
	string out;

	// Currently only valid for u-Blox devices
	if (_detectUblox) {
		if (!cfgOptions.size()) {
			cmd = baseCmd + "-h -v5 | awk '$1 ~ /CFG-.*-/ {print $1}'";
			out = System::ExecCommand(cmd);

			istringstream is(out);
			for (string line; std::getline(is, line); ) {
				if (line.empty())
					continue;

				if (line[line.length() - 1] == '\r')
					line.erase(line.length() - 1);

				cfgOptions.push_back(line);
			}
		}

		routeable_message rMsg;
		messages::GPSDUBXConfigMessage cfgArray;
		for (string opt: cfgOptions) {
			string val;
			if (GetConfigValue(opt, val, &_dataLock)) {
				messages::GPSDUBXConfigMessage cfg;
				cfg.set_parameter(opt);
				cfg.set_value(val);

				cfgArray.add_array_element(cfgArray.ArrayElement, cfg);
			}
		}
		rMsg.initialize(cfgArray, "Internal");
		this->IncomingMessage(rMsg);
	}
}

const char *ToString(FixTypes fix)
{
	switch (fix)
	{
	case FixTypes::NoFix: return FIXTYPES_NOFIX_STRING;
	case FixTypes::TwoD: return "2D";
	case FixTypes::ThreeD: return "3D";
	default: return FIXTYPES_UNKNOWN_STRING;
	}
}

const char *ToString(SignalQualityTypes quality)
{
	switch (quality)
	{
	case SignalQualityTypes::GPS: return SIGNALQUALITYTYPES_GPS_STRING;
	case SignalQualityTypes::DGPS: return SIGNALQUALITYTYPES_DGPS_STRING;
	case SignalQualityTypes::PPS: return SIGNALQUALITYTYPES_PPS_STRING;
	case SignalQualityTypes::RealTimeKinematic: return "RTK";
	case SignalQualityTypes::FloatRTK: return SIGNALQUALITYTYPES_FLOATRTK_STRING;
	case SignalQualityTypes::DeadReckoning: return SIGNALQUALITYTYPES_DEADRECKONING_STRING;
	case SignalQualityTypes::ManualInputMode: return SIGNALQUALITYTYPES_MANUALINPUTMODE_STRING;
	case SignalQualityTypes::SimulationMode: return SIGNALQUALITYTYPES_SIMULATIONMODE_STRING;
	default: return SIGNALQUALITYTYPES_INVALID_STRING;
	}
}

void BroadcastLocation(LocationMessage &loc, LocationPlugin *plugin) {
	// This function pieces together disjoint parts of the location message
	static LocationMessage _tmp;
	if (loc.get_Id() == "") {
		_tmp = loc;
		return;
	} else if (!_detectDGPS) {
		// Reset the quality measure until new data is needed
		_tmp.set_SignalQuality(loc.get_SignalQuality());
	}

	if (loc.get_SignalQuality() < _tmp.get_SignalQuality())
		loc.set_SignalQuality(_tmp.get_SignalQuality());

	plugin->BroadcastMessage(loc);
}

/**
 * There are three separate worker threads for GPSD.
 * Note that each require separate types of streams from the GPSD server.
 * When adding them all to the same thread, the raw streams consume most
 * of the CPU, thus starving out the Location message generator. Therefore,
 * it was a better design to separate the connections with different sockets
 * running in different threads.
 *
 * 1) The Location message generator, which is required
 * 2) The NMEA message generator, when selected
 * 3) The RAW bytes message reader, for use in scanning proprietary
 *    u-Blox messages and for other stuff like RTCM
 */

template <>
void LocationPlugin::DoSampleGPS<WATCH_JSON>() {
	static uint64_t lastTime = 0;
	static string lastDev;

	static double _lastLat = 0.0;
	static double _lastLon = 0.0;
	static double _lastAlt = 0.0;
	static uint64_t _lastHeading = 0;

	auto &gps = GPSDConnection::connection(WATCH_JSON);
	if (gps.is_open()) {
		struct gps_data_t *gps_data = gps.get()->read();
		routeable_message rMsg;
		rMsg.get_source();

		if (gps_data) {
			// Decode the location data
			auto tm = Clock::GetMillisecondsSinceEpoch(gps_data->fix.time);

			if (tm > lastTime) {
				// Send the location message
				LocationMessage loc { Uuid::NewGuid(),
					get_quality(gps_data->fix.status), "", to_string(tm),
					gps_data->fix.latitude, gps_data->fix.longitude,
					get_fix(gps_data->fix.mode), gps_data->satellites_used,
					gps_data->dop.hdop, gps_data->fix.speed,
					gps_data->fix.track };

				loc.to_string();

				if (gps_data->set & ALTITUDE_SET) {
					loc.set_Altitude(gps_data->fix.altHAE);
				}

				_detect3DFix = (loc.get_FixQuality() >= FixTypes::ThreeD);

				if (loc.get_FixQuality() > FixTypes::NoFix) {
					_detectDGPS = (loc.get_SignalQuality() == SignalQualityTypes::DGPS);

					// Latch the heading and position if the vehicle slows below minimum speed
					// But, ignore latching for stationary
					if (loc.get_SignalQuality() == SignalQualityTypes::ManualInputMode) {
						// Do nothing
						;
					} else if (_latchSpeed > loc.get_Speed_mph()) {
						loc.set_Latitude(_lastLat);
						loc.set_Longitude(_lastLon);
						loc.set_Altitude(_lastAlt);
						loc.set_Heading((double)_lastHeading);
						loc.set_Speed_mps(0.0);
					} else {
						_lastLat = loc.get_Latitude();
						_lastLon = loc.get_Longitude();
						_lastAlt = loc.get_Altitude();
						_lastHeading = loc.get_Heading();
					}

					// Broadcast immediately
					BroadcastLocation(loc, this);
				} else {
					_lastLat = 0.0;
					_lastLon = 0.0;
					_lastAlt = 0.0;
					_lastHeading = 0;
				}

				if (!loc.is_empty() && _statusThrottle.Monitor(0x00)) {
					string tmStr = loc.get_Time();
					if (!tmStr.empty())
						SetStatus("Last GPS Time", Clock::ToLocalPreciseTimeString(Clock::GetTimepointSinceEpoch(atol(tmStr.c_str()))));

					SetStatus("Current Time", Clock::ToLocalPreciseTimeString(Clock::GetTimepointSinceEpoch(Clock::GetMillisecondsSinceEpoch())));
					//SetStatus("GPS", (ctrl.get_gps_device().empty() ? "N/A" :
					//	(ctrl.get_gps_type().empty() ? "Unknown type" : ctrl.get_gps_type()) + " on " + ctrl.get_gps_device()));
					SetStatus("Number of Satellites", loc.get_NumSatellites());
					SetStatus("Fix Quality", ToString(loc.get_FixQuality()));
					SetStatus("Signal Quality", ToString(loc.get_SignalQuality()));
					if (loc.get_FixQuality() > FixTypes::NoFix) {
						SetStatus("Altitude", loc.get_Altitude());
						SetStatus("Latitude", loc.get_Latitude(), false, 10);
						SetStatus("Longitude", loc.get_Longitude(), false, 10);
						SetStatus("Speed (MPH)", loc.get_Speed_mph());
						SetStatus("Heading", loc.get_Heading().get_value());
						SetStatus("HDOP", loc.get_HorizontalDOP(), false, 4);
					} else {
						SetStatus("Altitude", "N/A");
						SetStatus("Latitude", "N/A");
						SetStatus("Longitude", "N/A");
						SetStatus("Speed (MPH)", "N/A");
						SetStatus("Heading", "N/A");
						SetStatus("HDOP", "N/A");
					}
				}

				lastTime = tm;
			}

			// Decode the GPSD version data, if available
			if (gps_data->set & VERSION_SET) {
				messages::GPSDVersionMessage ver(gps_data->version.release);

				rMsg.refresh_timestamp();
				rMsg.initialize(ver, "Internal");
				rMsg.reinit();

				this->IncomingMessage(rMsg);
			}

			if (gps_data->set & RTCM2_SET) {
				_detectRtcm2 = true;
			}

			if (gps_data->set & RTCM3_SET) {
				_detectRtcm3 = true;
			}

			// Decode the GPSD device data
			for (size_t i = 0; lastDev != gps_data->dev.path && i < gps_data->devices.ndevices; i++) {
				if (strcmp(gps_data->dev.path, gps_data->devices.list[i].path) == 0) {
					messages::GPSDDeviceMessage dev;
					dev.set_path(gps_data->devices.list[i].path);
					dev.set_driver(gps_data->devices.list[i].driver);
					dev.set_flags(gps_data->devices.list[i].flags);
					dev.set_activated(Clock::GetMillisecondsSinceEpoch(gps_data->devices.list[i].activated));
					dev.set_subtype(gps_data->devices.list[i].subtype);
					dev.set_subtype1(gps_data->devices.list[i].subtype1);

					rMsg.refresh_timestamp();
					rMsg.initialize(dev, "Internal");
					rMsg.reinit();

					this->IncomingMessage(rMsg);
				}
			}

			lastDev = gps_data->dev.path;
		}
	}
}

template <>
void LocationPlugin::DoSampleGPS<WATCH_NMEA>() {
	// Send RAW NMEA strings
	auto &gps = GPSDConnection::connection(WATCH_NMEA);
	if (gps.is_open()) {
		gps.get()->read();

		istringstream is(gps.get()->data());
		for (string line; std::getline(is, line);) {
			if (line.empty())
				continue;

			if (line[line.length() - 1] == '\r')
				line.erase(line.length() - 1);
			FILE_LOG(logDEBUG3) << "Scanning '" << line << "'";

			TmxNmeaMessage msg;
			const string baseST = msg.get_subtype();

			msg.set_sentence(line);

			FILE_LOG(logDEBUG3) << "Sentence=" << msg.get_sentence()
					<< ",subtype=" << msg.get_subtype();

			if (!msg.get_subtype().empty() && msg.get_subtype() != baseST)
				this->OutgoingMessage(static_cast<const routeable_message &>(msg), true);
		}
	}
}

void decodeNAVPVT(string byteStr, LocationPlugin *plugin) {
	static string ubxNAVPVTPattern = "b56201075c00";
	static byte_stream ubxNAVPVTBytes = byte_stream_decode(ubxNAVPVTPattern);
	static size_t ubxNAVPVTLength = (size_t) ubxNAVPVTBytes[ubxNAVPVTBytes.size() - 2];

	regex exp(ubxNAVPVTPattern + "[a-zA-Z0-9]{" + ::to_string(ubxNAVPVTLength) + "}");
	regex_token_iterator<std::string::iterator> rend;
	regex_token_iterator<std::string::iterator> tokenizer { byteStr.begin(), byteStr.end(), exp };

	byte_stream msgBytes(ubxNAVPVTLength);

	for ( ; tokenizer != rend; tokenizer++) {
		// This uBlox message is used to check for RTK float or fixed,
		// so pass to the Location decoding thread
		messages::GPSDNAVPVTMessage navPvt;
		routeable_message rMsg;
		rMsg.initialize(navPvt, "Internal");
		msgBytes = byte_stream_decode(tokenizer->str());
		rMsg.set_payload_bytes(msgBytes);
		rMsg.reinit();

		FILE_LOG(logDEBUG1) << this_thread::get_id() << ": Sending: " << rMsg;
		plugin->IncomingMessage(rMsg, GPSD_GROUP, WATCH_JSON);
	}
}

bool isRtcmNew(RTCM2Message *msg) {
	static uint16_t lastCRC1 = 0;
	static uint16_t lastCRC2 = 0;

	if (!msg) return false;

	if (lastCRC1 == msg->get_Parity1() &&
		lastCRC2 == msg->get_Parity2()) {
		return false;
	}

	lastCRC1 = msg->get_Parity1();
	lastCRC2 = msg->get_Parity2();
	return true;
}

bool isRtcmNew(RTCM3Message *msg) {
	static uint32_t lastCRC;

	if (!msg) return false;

	if (lastCRC == msg->get_CRC())
		return false;

	lastCRC = msg->get_CRC();
	return true;
}

template <rtcm::RTCM_VERSION _Ver>
void decodeRtcm(const byte_stream &bytes, LocationPlugin *plugin) {
	static string rtcmByte;

	byte_stream msgBytes;

	int bytesRemaining = bytes.size();
	size_t i = 0;

	while (bytesRemaining > 0) {
		RTCMMessageType<_Ver, 0> rtcmMsg;

		if (rtcmByte.empty())
			rtcmByte.append(1, (char)rtcmMsg.get_Preamble());

		while (i < bytes.size() && bytes[i] != (byte_t)rtcmByte[0]) i++;

		if (i < bytes.size()) {
			msgBytes.resize(bytes.size() - i);
			memcpy(msgBytes.data(), &bytes.data()[i], msgBytes.size());

			FILE_LOG(logDEBUG3) << this_thread::get_id() << ": " << msgBytes;

			rtcmMsg.set_contents(msgBytes);

			FILE_LOG(logDEBUG2) << this_thread::get_id() << ": " << rtcmMsg;

			if (rtcmMsg.is_Valid()) {
				TmxRtcmEncodedMessage rtcmEnc;
				rtcmEnc.initialize(rtcmMsg);

				FILE_LOG(logDEBUG1) << this_thread::get_id() << ": Sending: " << rtcmEnc;
				plugin->OutgoingMessage(rtcmEnc, true);

				i += rtcmEnc.get_payload_bytes().size();
			} else {
				i++;
			}
		}

		bytesRemaining -= i;
	}
}

template <>
void LocationPlugin::DoSampleGPS<WATCH_RAW>() {
	static string rawString;

	byte_stream bytes(4096);
	byte_stream msgBytes;

	auto &gps = GPSDConnection::connection(WATCH_RAW);
	if (gps.is_open()) {
		int fd = gps.get_fd();
		int r = ::read(fd, bytes.data(), bytes.size());
		if (r > 0) {
			if (r < bytes.size()) bytes.resize(r);

			rawString.append(bytes.begin(), bytes.end());

			// Go up to the last newline
			size_t lastNL = rawString.find_last_of('\n');
			size_t len = rawString.length() - (lastNL < rawString.length() ? lastNL : 0);
			bytes.resize(len);
			memcpy(bytes.data(), rawString.c_str(), len);
			if (len == rawString.length())
				rawString.clear();
			else
				rawString.erase(rawString.begin(), rawString.begin() + len);

			// Notify worker threads of newly received bytes
			messages::GPSDDecodeRawMessage rawMsg;
			routeable_message rMsg;
			rMsg.initialize(rawMsg, "Internal");
			rMsg.set_payload_bytes(bytes);
			rMsg.reinit();

			PLOG(logDEBUG1) << this_thread::get_id() << ": Sending: " << rMsg;
			this->IncomingMessage(rMsg);
		}
	}
}

template <uint32_t _I>
void doSample(uint16_t connection, LocationPlugin *_plugin) {
	if (_I == connection) {
		_plugin->DoSampleGPS<_I>();
	} else {
		doSample<_I * 2>(connection, _plugin);
	}
}

template <>
void doSample<0x10000u>(uint16_t connection, LocationPlugin *_plugin) { }

size_t lastSampled(byte_t id) {
	static map<size_t, uint64_t> lastSampled;
	if (!lastSampled.count(id))
		lastSampled[id] = 0;

	return lastSampled[id];
}

bool LocationPlugin::Accept(byte_t groupId, byte_t uniqId) {
	if (groupId != GPSD_GROUP)
		return TmxMessageManager::Accept(groupId, uniqId);

	return _sampleThrottle.Monitor(uniqId);
}

void LocationPlugin::Cleanup(byte_t groupId, byte_t uniqId) {
	if (groupId != GPSD_GROUP)
		return TmxMessageManager::Cleanup(groupId, uniqId);
}

/**
 * Decode and process the incoming messages. This function deals mainly
 * with the internal GPSD messages, and passes the others on up the chain
 */
void LocationPlugin::OnMessageReceived(const routeable_message &msg) {
	auto ivpMsg = msg.get_message();
	if (!ivpMsg) return;

	routeable_message rMsg(ivpMsg);
	rMsg.flush();
	FILE_LOG(logDEBUG1) << this_thread::get_id() << ": Received " << rMsg;

	if (IsMessageOfType<messages::GPSDSampleMessage>(ivpMsg)) {
		messages::GPSDSampleMessage sample = rMsg.get_payload<messages::GPSDSampleMessage>();
		doSample<1>(sample.get_connectionId(), this);
	} else if (IsMessageOfType<messages::GPSDVersionMessage>(ivpMsg)) {
		messages::GPSDVersionMessage ver = rMsg.get_payload<messages::GPSDVersionMessage>();
		FILE_LOG(logINFO) << "Connected to GPSD version " << ver.get_version();
	} else if (IsMessageOfType<messages::GPSDDeviceMessage>(ivpMsg)) {
		messages::GPSDDeviceMessage dev = rMsg.get_payload<messages::GPSDDeviceMessage>();

		string driver = dev.get_driver();

		// Need to configure the device if the driver was newly discovered as a u-Blox
		if (!_detectUblox.exchange((driver == "u-blox")))
			ConfigureDevice();

		if (_statusThrottle.Monitor(0xDEF1CE)) {
			string type = dev.get_subtype1();
			if (!type.empty()) {
				auto loc = type.find("MOD=");
				if (loc != string::npos) {
					type = type.substr(loc+4);
					loc = type.find(",");
					if (loc != string::npos) {
						type = type.substr(0, loc);
					}
				}

				type = " " + type;
			}
			SetStatus("GPS Device Type", driver + (type.empty() ? "" : type));
			SetStatus("GPS Device Path", dev.get_path());
			SetStatus("Connected Since", Clock::ToLocalPreciseTimeString(Clock::GetTimepointSinceEpoch(dev.get_activated())));
		}
	} else if (IsMessageOfType<messages::GPSDUBXConfigMessage>(ivpMsg)) {
		string cmd = baseCmd;
		string host = GPSDConnection::connection(WATCH_JSON).get_host();

		messages::GPSDUBXConfigMessage cfgArray = rMsg.get_payload<messages::GPSDUBXConfigMessage>();
		for (messages::GPSDUBXConfigMessage cfg: cfgArray.get_array<messages::GPSDUBXConfigMessage>(cfgArray.ArrayElement)) {
			cmd.append("-z ");
			cmd.append(cfg.get_parameter());
			cmd.append(",");
			cmd.append(cfg.get_value());
			cmd.append(" ");
		}

		if (cmd != baseCmd) {
			cmd += host;
			PLOG(logDEBUG) << "Configuring u-Blox device with " << cmd;
			System::ExecCommand(cmd);

			PLOG(logDEBUG) << "Saving u-Blox device configuration with " << cmd;
			cmd = baseCmd + " -p SAVE " + host;
			string out = System::ExecCommand(cmd);
			PLOG(logDEBUG) << out;
		}
	} else if (IsMessageOfType<messages::GPSDNAVPVTMessage>(ivpMsg)) {
		// For a u-blox RTK device, the RTK float and fixed-integer modes are reported as DGPS.
		// Need to check the UBX-NAV-PVT message flags for the carrier signal state
		// See the Interface Description document at www.u-blox.com
		SignalQualityTypes quality = SignalQualityTypes::Invalid;

		// Extract the message and decode the flags
		byte_stream bytes = rMsg.get_payload_bytes();
		if (bytes.size() >= 28) {
			byte_t b = bytes[27];
			if ((b & 0x40) == 0x04)
				quality = SignalQualityTypes::FloatRTK;
			else if ((b & 0x80) == 0x08)
				quality = SignalQualityTypes::RealTimeKinematic;
			else if ((b & 0x02) == 0x02)
				quality = SignalQualityTypes::DGPS;
		}

		if (quality > SignalQualityTypes::Invalid) {
			LocationMessage tmp;
			tmp.set_SignalQuality(quality);
			BroadcastLocation(tmp, this);
		}
	} else if (IsMessageOfType<messages::GPSDDecodeRawMessage>(ivpMsg)) {
		byte_stream bytes = rMsg.get_payload_bytes();

		if (_detectRtcm3)
			decodeRtcm<rtcm::RTCM_VERSION::SC10403_3>(bytes, this);
		if (_detectRtcm2)
			decodeRtcm<rtcm::RTCM_VERSION::SC10402_3>(bytes, this);
		if (_detectDGPS && _detectUblox)
			decodeNAVPVT(rMsg.get_payload_str(), this);

	} else {
		// Pass it up the chain
		TmxMessageManager::OnMessageReceived(msg);
	}
}

void LocationPlugin::SampleGPS() {
	int r;
	struct timeval tv;

	// We want to force each sampling into a separate
	// thread. However, the assignments for the NMEA
	// and the Raw thread may not happen immediately,
	// so "prime" the assignment with a dummy message.
	// These are remembered because we do not clear
	// assignments in the group
	static bool firstTime = true;
	if (firstTime) {
		this->IncomingMessage("Primer", NULL, GPSD_GROUP, WATCH_JSON);
		this->IncomingMessage("Primer", NULL, GPSD_GROUP, WATCH_NMEA);
		this->IncomingMessage("Primer", NULL, GPSD_GROUP, WATCH_RAW);

		this_thread::sleep_for(_sampleThrottle.get_Frequency());
		firstTime = false;
	}

	fd_set fds;
	FD_ZERO(&fds);

	tv.tv_sec = 0;
	tv.tv_usec = chrono::milliseconds(_sampleThrottle.get_Frequency()).count();

	std::bitset<16> mask(WATCH_JSON);
	if (_sendNmea)
		mask |= WATCH_NMEA;
	else
		GPSDConnection::connection(WATCH_NMEA).close();

	if (_detectRtcm2 || _detectRtcm3)
		mask |= WATCH_RAW;
	else if (_detectUblox && _detectDGPS)
		mask |= WATCH_RAW;

	int nfds = 0;
	for (size_t i = 0; i < mask.size(); i++) {
		int connType = pow(2, i);
		auto &conn = GPSDConnection::connection(connType);

		if (!mask[i]) {
			// Close any open connection to GPSd
			if (conn.is_open())
				conn.close();
			continue;
		}

		if (conn.get_host() != _gpsdHost) {
			lock_guard<mutex> lock(_dataLock);
			if (conn.open(_gpsdHost) && conn.get_id() == WATCH_JSON) {
				FILE_LOG(logINFO) << "Connected to GPSD on " <<
					conn.get_host() << ":" << DEFAULT_GPSD_PORT;
			}
		}

		if (conn.is_open()) {
			int fd = conn.get_fd();
			FD_SET(fd, &fds);

			nfds = max(nfds, fd);
		}
	}

	// Look for new bytes available on any of the open streams
	r = ::select(nfds+1, &fds, NULL, NULL, &tv);
	if (r > 0) {
		for (size_t i = 0; i < mask.size(); i++) {
			if (!mask[i]) continue;

			int connType = pow(2, i);

			auto &conn = GPSDConnection::connection(connType);
			if (conn.is_open()) {
				int fd = conn.get_fd();

				if (FD_ISSET(fd, &fds)) {
					// Send notification to process
					routeable_message rMsg;
					messages::GPSDSampleMessage sample(connType);
					rMsg.initialize(sample, "Internal");
					this->IncomingMessage(rMsg, GPSD_GROUP, connType);
				}
			}
		}
	} else if (r < 0 && errno != EINTR) {
		stringstream ss;
		ss << "Unable to select on file descriptors: " << strerror(errno);
		TmxException ex(ss.str());
		this->HandleException(ex, true);
	}

	this_thread::sleep_for(_sampleThrottle.get_Frequency());
}

} /* End namespace LocationPlugin */

