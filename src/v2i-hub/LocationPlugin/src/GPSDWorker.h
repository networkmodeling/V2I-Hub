/*
 * GPSDWorker.h
 *
 *  Created on: May 14, 2020
 *      @author: Gregory M. Baumgardner
 */

#ifndef GPSDWORKER_H_
#define GPSDWORKER_H_

#include "LocationPlugin.h"

#include <gps.h>
#include <libgpsmm.h>

#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include <ThreadWorker.h>
#include <tmx/messages/routeable_message.hpp>

namespace LocationPlugin {

class GPSDConnection {
public:
	virtual ~GPSDConnection() { close(); }

	std::string get_host() const { return _host; }
	uint32_t get_id() const { return _id; }
	int get_fd() const { return _fd; }
	std::shared_ptr<gpsmm> get() { return _ptr; }
	bool is_open() const { return _ptr && _ptr->is_open(); }
	void close() { _ptr.reset(); _fd = 0; _host = ""; }

	bool open(std::string host) {
		if (host.empty()) return false;

		if (_host != host)
			close();

		if (!_ptr) {
			_host = host;

			_ptr.reset(new gpsmm(host.c_str(), DEFAULT_GPSD_PORT));
			if (is_open()) {
				gps_data_t *gps_data = _ptr->stream(WATCH_ENABLE | get_id());
				if (gps_data && gps_data->gps_fd > 0)
					_fd = gps_data->gps_fd;
			}
		}

		return is_open();
	}

	static GPSDConnection &connection(uint32_t id = WATCH_JSON) {
		// This function must be run sequentially
		static std::mutex _connLock;
		std::lock_guard<std::mutex> lock(_connLock);

		static vector<unique_ptr<GPSDConnection> > connections;

		size_t idx;
		for (idx = 0; idx < connections.size(); idx++) {
			if (id == connections[idx]->get_id())
				break;
		}

		if (idx == connections.size()) {
			connections.emplace_back(new GPSDConnection(id));
		}

		return *(connections[idx]);
	}
private:
	GPSDConnection(uint32_t id): _id(id) { }
	GPSDConnection(const GPSDConnection &other):
		_host(other.get_host()), _id(other.get_id()) { }

	std::string _host;
	uint32_t _id = 0;
	int _fd = 0;
	std::shared_ptr<gpsmm> _ptr;
};

namespace messages {

/**
 * GPSD Message types
 */

class GPSDMessage: public tmx::message {
public:
	static constexpr const char *MessageType = "GPSD";

	GPSDMessage(): tmx::message() { }
	GPSDMessage(const message &other): tmx::message(other) { }
};

class GPSDSampleMessage: public GPSDMessage {
public:
	static constexpr const char *MessageSubType = "Sample";

	GPSDSampleMessage(): GPSDMessage() { }
	GPSDSampleMessage(uint32_t connId): GPSDMessage() {
		this->set_connectionId(connId);
	}

	std_attribute(this->msg, uint32_t, connectionId, 0, )
};

class GPSDVersionMessage: public GPSDMessage {
public:
	static constexpr const char *MessageSubType = "Version";

	GPSDVersionMessage(): GPSDMessage() { }
	GPSDVersionMessage(std::string ver): GPSDMessage() {
		this->set_version(ver);
	}

	std_attribute(this->msg, std::string, version, "", );
};

class GPSDDeviceMessage: public GPSDMessage {
public:
	static constexpr const char *MessageSubType = "Device";

	GPSDDeviceMessage(): GPSDMessage() { }

	std_attribute(this->msg, std::string, path, "", );
	std_attribute(this->msg, std::string, driver, "", );
	std_attribute(this->msg, int, flags, 0, );
	std_attribute(this->msg, uint64_t, activated, 0, );
	std_attribute(this->msg, std::string, subtype, "", );
	std_attribute(this->msg, std::string, subtype1, "", );
};

class GPSDUBXConfigMessage: public GPSDMessage {
public:
	static constexpr const char *MessageSubType = "Config";
	static constexpr const char *ArrayElement = "configs";

	GPSDUBXConfigMessage(): GPSDMessage() { }
	GPSDUBXConfigMessage(const message &other): GPSDMessage(other) { }

	std_attribute(this->msg, std::string, parameter, "", );
	std_attribute(this->msg, std::string, value, "", );

	static message_tree_type to_tree(GPSDUBXConfigMessage &msg) {
		return tmx::message::to_tree(msg);
	}

	static GPSDUBXConfigMessage from_tree(const message_tree_type &tree) {
		return GPSDUBXConfigMessage(tmx::message::from_tree(tree));
	}
};

class GPSDNAVPVTMessage: public GPSDMessage {
public:
	static constexpr const char *MessageSubType = "NAVPVT";
};

class GPSDDecodeRawMessage: public GPSDMessage {
public:
	static constexpr const char *MessageSubType = "DecodeRaw";
};

} /* End namespace messages */
} /* End namespace LocationPlugin */

#endif /* GPSDWORKER_H_ */
