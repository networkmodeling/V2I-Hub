/*
 * RtcmBaseMessage.h
 *
 *  Created on: Mar 23, 2018
 *      Author: gmb
 */

#ifndef INCLUDE_RTCMMESSAGE_H_
#define INCLUDE_RTCMMESSAGE_H_

#include <bitset>
#include <map>

#include <tmx/TmxException.hpp>
#include <tmx/messages/routeable_message.hpp>
#include <tmx/TmxApiMessages.h>

#include "rtcm/RtcmTypes.h"

namespace tmx {
namespace messages {

#define ALL_RTCM_VERSIONS SC(2,3) SC(3,3)
#define SC(X, Y) SC1040 ## X ## _ ## Y,
enum RTCM_VERSION {
	UNKNOWN = 0, ALL_RTCM_VERSIONS RTCM_EOF
};
#undef SC
#define SC(X, Y) quoted_attribute_name(X ## . ## Y),
static constexpr const char * const RTCM_VERSION_NAMES[] = { "Unknown", ALL_RTCM_VERSIONS };
#undef SC

struct RTCMData { };

/***
 * The basic class for an RTCM message.  This is a generic parent class that contains some
 * basic operations for all RTCM messages, but
 */

/*
class RTCMMessage: public tmx::message {
public:
	typedef rtcm::uint8 byte_type;

	static constexpr const char *MessageType = api::MSGSUBTYPE_RTCMCORRECTIONS_STRING;
	static constexpr const char *MessageSubType = RTCM_VERSION_NAMES[RTCM_VERSION::UNKNOWN];

	RTCMMessage(): tmx::message() {	}
	RTCMMessage(const tmx::byte_stream &in): _encodedBytes(in) { }
	virtual ~RTCMMessage() { }

	// Virtual functions for all RTCM messages
	virtual uint32_t get_MessageNumber() { return 0; };
	virtual RTCM_VERSION get_RTCMVersion() { return RTCM_VERSION::UNKNOWN; };
	virtual tmx::byte_stream encoded_bytes() { return _encodedBytes; }
	virtual tmx::byte_stream decoded_bytes() { return _decodedBytes; }

	void set_Version(RTCM_VERSION version)
protected:
	// This is meant to be the encoded bytes
	tmx::byte_stream _encodedBytes;
	tmx::byte_stream _decodedBytes;

	// Universal encode/decode functions
	template <typename T>
	size_t consume(size_t start, const T t) {
		// Assume a standard attribute is being used for an RTCM type
		typedef typename T::traits_type::attr_type rtcm_type;
		size_t bitsToConsume = rtcm_type::size;

		// Convert the bytes to a value
		size_t startByte = start / byte_type::size;
		size_t end = (start + bitsToConsume);

		typename rtcm_type::data_type count = 0;
		size_t bits = startByte * byte_type::size;
		for (size_t byte = startByte; byte < _decodedBytes.size() && bits < end; byte++, bits += byte_type::size) {
			if (byte > startByte) count <<= byte_type::size;
			count += _decodedBytes[byte];
		}

		// Mask the correct bits of the value
		typename rtcm_type::data_type mask = (1 << bitsToConsume) - 1;
		mask <<= (bits - end);

		count = ((count & mask) >> (bits - end));
		t.set(this->msg, count);

		return start + rtcm_type::size;
	}

	template <typename T, typename... Args>
	size_t consume(size_t start, const T t, const Args... args) {
		return consume<T, Args...>(consume<T>(start, t), args...);
	}

	template <RTCM_VERSION Version>
	void decode_data() { // TODO Exception }

	// Some common helper functions
	template <typename Traits>
	typename Traits::word_type decode_word(size_t wordNum = 0) {
		typename Traits::word_type val;

		word_size_check<Traits>();

		size_t byteIncr = Traits::WordSize / Traits::UsableBits;
		for (size_t i = 0, idx = wordNum * byteIncr + i; i < byteIncr && idx < _encodedBytes.size(); i++, idx++) {
			if (i > 0)
				val = (val << Traits::UsableBits);

			val  |= roll<Traits>((_bytes[idx] & 0x40) == 0x40 ?
					_bytes[idx] : 0x00);
		}

		return val;
	}

	template <typename Traits>
	std::vector<typename Traits::word_type> decode_header() {
		std::vector<Traits::word_type> header;
		for (size_t i = 0; i < Traits::HeaderWords; i++)
			header.push_back(decode_word<Traits>(i));
		return header;
	}

	template <typename Traits>
	std::vector<typename Traits::word_type> decode_data() {
		std::vector<Traits::word_type> data;
		for (size_t i = Traits::HeaderWords; i < _encodedBytes.size(); i++)
			data.push_back(decode_word<Traits>(i));
		return data;
	}

	template <typename Traits>
	byte_t roll(const byte_t byte) {
		std::bitset<Traits::UsableBits> val;
		for (size_t i = 0; i < val.size(); i++) {
			size_t shift = val.size() - i - 1;
			val[i] = (byte & (1 << shift)) == (1 << shift);
		}
		return (byte_t) val.to_ulong();
	}

private:
	RTCM_VERSION _version;

	template <size_t a, size_t b>
	struct size_check {
		static_assert(a <= b, "Need a bigger data type to hold a RTCM word");
	};

	template <typename Traits>
	static void word_size_check() {
		static struct size_check<Traits::WordSize, 8 * sizeof(Traits::word_type)> _word_size_check;
	};
};

template <RTCM_VERSION Version>
class RTCMMessageType: public RTCMMessage {
public:
	static constexpr RTCM_VERSION RTCM_Version = Version;

	using RTCMMessage::MessageType;
	static constexpr const char *MessageSubType = RTCM_VERSION_NAMES[RTCM_Version];

	RTCMMessageType(): RTCMMessage() {	}

	RTCMMessageType(const tmx::byte_stream &in): RTCMMessage(in) { }
};

class RTCMEncodedMessage: public tmx::routeable_message {
public:
	RTCMEncodedMessage(): tmx::routeable_message() { }
	RTCMEncodedMessage(const tmx::routeable_message &other):
		tmx::routeable_message(other) { this->reinit();	}
	virtual ~RTCMEncodedMessage() { }

	RTCMMessage get_rtcm_payload() {
		std::cout << "Getting RTCM payload" << " with " << this->get_encoding() << " and " << this->get_payload_bytes() << std::endl;

		// Start with the last known version of RTCM and work backwards
		static constexpr const RTCM_VERSION lastVersion = (RTCM_VERSION)((int)RTCM_VERSION::RTCM_EOF - 1);

		if (!_decoded && this->get_encoding() == api::ENCODING_BYTEARRAY_STRING)
			this->decode<lastVersion>(this->get_subtype().c_str(), this->get_payload_bytes());
		else
			return tmx::routeable_message::get_payload<RTCMMessage>();

		return *_decoded;
	}

	template <RTCM_VERSION Version>
	void set_rtcm_payload(RTCMMessageType<Version> &msg) {
		_decoded.reset();
		this->set_payload_bytes(msg.encoded_bytes());
	}

	template <RTCM_VERSION Version>
	void initialize(RTCMMessageType<Version> &msg, const std::string source = "", unsigned int sourceId = 0, unsigned int flags = 0) {
		tmx::routeable_message::initialize(RTCMMessage::MessageType, RTCMMessageType<Version>::MessageSubType,
				source, sourceId, flags);
		this->set_rtcm_payload(msg);
	}

private:
	std::unique_ptr<RTCMMessage> _decoded;

	// Recursive template to set the correct RTCM message type
	template <RTCM_VERSION Version>
	void decode(const char *version, const tmx::byte_stream &bytes) {
		if (version && strcmp(RTCMMessageType<Version>::MessageSubType, version) == 0) {
			_decoded.reset(new RTCMMessageType<Version>(bytes));
		} else {
			static constexpr const RTCM_VERSION next = (RTCM_VERSION)((int)Version - 1);
			decode<next>(version, bytes);
		}
	}
};

// Template specialization for decoding an unknown version
template <>
void RTCMEncodedMessage::decode<RTCM_VERSION::UNKNOWN>(const char *version, const tmx::byte_stream &bytes) {
	// If this was expected to be a known version, then this case is an error
	if (version && strcmp(RTCMMessage::MessageSubType, version) != 0) {
		std::stringstream err;
		err << "Unable to decode bytes.  ";
		err << "RTCM Version " << version << " does not exist.";

		tmx::TmxException ex(err.str().c_str());
		BOOST_THROW_EXCEPTION(ex);
	}

	// Set the unknown RTCM message
	_decoded.reset(new RTCMMessage(bytes));
}
*/
} /* End namespace messages */
} /* End namespace tmx */


//typedef DEFAULT_RTCM_MESSAGE RTCM_Message;


#endif /* INCLUDE_RTCMMESSAGE_H_ */
