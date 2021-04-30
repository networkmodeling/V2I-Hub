/*
 * RTCMSC10402_3Message.hpp
 *
 *  Created on: Apr 10, 2018
 *      Author: gmb
 */

#ifndef INCLUDE_RTCM_RTCMSC10402_3MESSAGE_HPP_
#define INCLUDE_RTCM_RTCMSC10402_3MESSAGE_HPP_

#include "../RTCMMessage.h"

namespace tmx {
namespace messages {

template <>
class RTCMMessage<SC10402_3>: public tmx::message {
public:
	enum StationHealthValue {
		UDREScaleFactor1_0 = 0,
		UDREScaleFactor0_75,
		UDREScaleFactor0_5,
		UDREScaleFactor0_3,
		UDREScaleFactor0_2,
		UDREScaleFactor0_1,
		NotMontitored,
		NotWorking
	};

	static constexpr const size_t WordSize = 30;
	static constexpr const size_t NumberHeaderWords = 2;
	static constexpr const size_t UsableBits = 6;
	static constexpr const size_t WordBytes = WordSize / UsableBits;
	static constexpr const size_t HeaderBits = NumberHeaderWords * WordSize;
	static constexpr const size_t HeaderBytes = NumberHeaderWords * WordBytes;

	typedef uint32_t word_type;
	typedef std::bitset<WordSize> word;

	static constexpr const RTCM_VERSION Version = SC10402_3;
	static constexpr const char *MessageType = RTCMMessage<UNKNOWN>::MessageType;
	static constexpr const char *MessageSubType = RTCM_VERSION_NAMES[Version];

	RTCMMessage<SC10402_3>(): tmx::message(), bytes(HeaderBytes) {
		this->get_Preamble();
	}

	template <RTCM_VERSION V>
	RTCMMessage<SC10402_3>(const RTCMMessage<V> &other): tmx::message(other.get_container()) { }

	RTCMMessage<SC10402_3>(const tmx::byte_stream &in): tmx::message(), bytes(in) {
		if (bytes.size() < HeaderBytes)
			bytes.resize(HeaderBytes);

		word_type tmp = 0;
		size_t count = WordSize;
#define CONSUME(X) count -= X::size(); if (count <= 0) count = WordSize; this->attr_func_name(set_, X)((typename X::data_type)mask<X>((tmp >> count)))
		// Consume the header data, one word at a time
		word_type tmp = read_word(bytes, 0);

		CONSUME(Preamble);
		CONSUME(FrameID);
		CONSUME(StationID);
		CONSUME(Parity1);
		CONSUME(ModifiedZCount);
		CONSUME(SequenceNumber);
		CONSUME(NumberDataWords);
		CONSUME(StationHealth);
		CONSUME(Parity2);
#undef CONSUME

		// Resize to the correct number of words
		bytes.resize(this->size());

		this->set_array("data", this->data_words());
	}

	virtual ~RTCMMessage<SC10402_3>() { }

#define bitset_attribute_builder(T, S, X, Y, D) \
public: \
	struct Y { \
		typedef X data_type; \
		typedef std::bitset<S> bitset_type; \
		static data_type default_value() { return D; } \
		static size_t size() { static bitset_type t; return t.size(); } \
	}; \
private: \
	T attr_field_name(Y);
#define ro_bitset_attribute(C, S, X, Y, D) \
	bitset_attribute_builder(battelle::attributes::standard_attribute<Y>, S, X, Y, D) \
	typesafe_getter_builder(C, X, Y, get_)
#define bitset_attribute(C, S, X, Y, D) \
	ro_bitset_attribute(C, S, X, Y, D) \
	typesafe_setter_builder(C, X, Y, set_, std::bitset<S> v(value); if (v.to_ulong() == value))

	ro_bitset_attribute(this->msg, 8, uint16_t, Preamble, 0x66)

	bitset_attribute(this->msg, 6, uint16_t, FrameID, 0)
	bitset_attribute(this->msg, 10, uint16_t, StationID, 0)
	bitset_attribute(this->msg, 6, uint16_t, Parity1, 0)
	bitset_attribute(this->msg, 13, uint16_t, ModifiedZCount, 0)
	bitset_attribute(this->msg, 3, uint16_t, SequenceNumber, 0)
	bitset_attribute(this->msg, 5, uint16_t, NumberDataWords, 0)
	bitset_attribute(this->msg, 3, StationHealthValue, StationHealth, NotWorking)
	bitset_attribute(this->msg, 6, uint16_t, Parity2, 0)

public:
	size_t size() {
		return HeaderBytes + (this->get_NumberDataWords() * WordSize / UsableBits);
	}

	tmx::byte_stream encoded_bytes() {
		header_type tmp = 0;
		size_t count = HeaderBits;
#define EXTRACT(X) count -= X::size(); tmp |= ((header_type)this->attr_func_name(get_, X)() << count)
		EXTRACT(Preamble);
		EXTRACT(FrameID);
		EXTRACT(StationID);
		EXTRACT(Parity1);
		EXTRACT(ModifiedZCount);
		EXTRACT(SequenceNumber);
		EXTRACT(NumberDataWords);
		EXTRACT(StationHealth);
		EXTRACT(Parity2);
#undef EXTRACT

		bytes.erase(bytes.begin(), bytes.begin() + HeaderBytes);
		for (size_t i = 0; i < HeaderBytes; i++) {
			if (i > 0)
				tmp = (tmp >> UsableBits);

			bytes.insert(bytes.begin(), (byte_t)(0x40 | roll(tmp & 0x3F)));
		}

		return bytes;
	}

	std::vector<word_type> data_words() {

		std::vector<word_type> words(this->get_NumberDataWords(), (word_type)0);

		for (size_t i = 0; i < words.size(); i++) {
			for (size_t j = 0; j < WordBytes; j++) {
				const size_t idx = HeaderBytes + (WordBytes * i + j);
				if (idx > bytes.size())
					break;

				if (j > 0)
					words[i] = (words[i] << UsableBits);

				words[i] |= roll(((bytes[idx] & 0x40) == 0x40) ?
						bytes[idx] : 0x00);
			}
		}

		return words;
	}

private:
	tmx::byte_stream bytes;

	void set_Preamble(uint16_t value) { }

	template <typename T>
	uint16_t mask(const word_type theValue) {
		typename T::bitset_type mask(0xFFFFFFFF);
		return theValue & mask.to_ulong();
	}

	byte_t roll(const byte_t byte) {
		std::bitset<UsableBits> val;
		for (size_t i = 0; i < val.size(); i++) {
			size_t shift = val.size() - i - 1;
			val[i] = (byte & (1 << shift)) == (1 << shift);
		}
		return (byte_t) val.to_ulong();
	}

	word_type read_word(const byte_stream &in, const size_t idx = 0) {
		word_type tmp = 0;

		for (size_t i = 0; i < WordBytes && i + idx < in.size(); i++) {
			if (i > 0)
				tmp = (tmp << UsableBits);

			tmp  |= roll((bytes[i+idx] & 0x40) == 0x40 ?
					bytes[i+idx] : 0x00);
		}

		return tmp;
	}


};

typedef RTCMMessage<SC10402_3> RTCMSC10402_3Message;

#define DEFAULT_RTCM_MESSAGE tmx::messages::RTCMSC10402_3Message

} /* End namespace messages */

template <>
template <>
inline messages::RTCMSC10402_3Message routeable_message::get_payload<messages::RTCMSC10402_3Message>() {
	messages::RTCMEncodedMessage encMsg(*this);
	return encMsg.get_rtcm_payload<messages::RTCMSC10402_3Message::Version>();
}

} /* End namespace tmx */



#endif /* INCLUDE_RTCM_RTCMSC10402_3MESSAGE_HPP_ */
