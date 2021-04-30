/*
 * DecodedBsmMessage.h
 *
 *  Created on: Jun 9, 2016
 *      Author: ivp
 */

#ifndef INCLUDE_DECODEDBSMMESSAGE_H_
#define INCLUDE_DECODEDBSMMESSAGE_H_

#if __cplusplus > 199711L
	#include <tmx/messages/message.hpp>
	#include <tmx/TmxApiMessages.h>
	#include "MessageTypes.h"
#else
	#include <tmx/messages/faux_message.hpp>
#endif

#include "Measurement.h"

namespace tmx {
namespace messages {

/**
 * A decoded representation of a Basic Safety Message.
 */
class DecodedBsmMessage : public tmx::message
{
	typedef Measurement<units::Angle, units::Angle::deg> degMeas;
	typedef Measurement<units::Distance, units::Distance::m> mMeas;
	typedef Measurement<units::Speed, units::Speed::mps> mpsMeas;
public:
	DecodedBsmMessage() {}

#if __cplusplus > 199711L
	/// Message type for routing this message through TMX core.
	static constexpr const char* MessageType = MSGTYPE_DECODED_STRING;

	/// Message sub type for routing this message through TMX core.
	static constexpr const char* MessageSubType = MSGSUBTYPE_BASIC_STRING;
#endif

	/// The message count in the range 0 to 127.
	/// This value should change when the vehicle ID changes or the data changes.
	std_attribute(this->msg, uint8_t, MsgCount, 0, )

	/// Temporary ID of the sending device.  It may change for anonymity.
	std_attribute(this->msg, int32_t, TemporaryId, 0, )

	/// The latitude of the sending device.
	std_attribute(this->msg, double, Latitude, 0.0, )

	/// The longitude of the sending device.
	std_attribute(this->msg, double, Longitude, 0.0, )

	/// The geographic position above or below the reference ellipsoid (typically WGS-84)
	/// The valid range is -409.5 to 6143.9 meters.
	std_attribute(this->msg, double, Elevation_m, 0.0, )

	/// The speed in meters per second
	std_attribute(this->msg, double, Speed, 0.0, )

	/// The current heading in degrees.
	/// The valid range is 0 to 359.9875 degrees.
	std_attribute(this->msg, double, Heading, 0.0, )

	/// The current angle of the steering wheel.
	/// The valid range is -189 to 189 degrees.
	std_attribute(this->msg, double, SteeringWheelAngle, 0.0, )

	/// Represents the millisecond within a minute, with a range of 0 - 60999.
	/// A leap second is represented by the value range 60000 to 60999.
	/// The value of 65535 represents an unavailable value in the range of the minute.
	std_attribute(this->msg, uint16_t, SecondMark, 0, )

	/// True if this is an outgoing message being routed to the DSRC radio.
	std_attribute(this->msg, bool, IsOutgoing, false, )

	/// True if Latitude and Longitude contain valid values.
	std_attribute(this->msg, bool, IsLocationValid, false, )

	/// True if Elevation contains a valid value.
	std_attribute(this->msg, bool, IsElevationValid, false, )

	/// True if Speed_mph contains a valid value.
	std_attribute(this->msg, bool, IsSpeedValid, false, )

	/// True if Heading contains a valid value.
	std_attribute(this->msg, bool, IsHeadingValid, false, )

	/// True if SteeringWheelAngle contains a valid value.
	std_attribute(this->msg, bool, IsSteeringWheelAngleValid, false, )

	/// Safe method to increment message count and keep it within the valid range.
	inline void IncrementMsgCount()
	{
		uint8_t value = get_MsgCount();
		value++;
		if (value > 127)
			value = 0;
		set_MsgCount(value);
	}

	/**
	 * @return The elevation with units
	 */
	inline mMeas get_Elevation_meas() {
		return mMeas(this->get_Elevation_m());
	}

	/**
	 * @return The speed with units
	 */
	inline mpsMeas get_Speed_meas() {
		return mpsMeas(this->get_Speed());
	}

	/**
	 * @return The heading with units
	 */
	inline degMeas get_Heading_meas() {
		return degMeas(this->get_Heading());
	}

	/**
	 * @return The steering wheel angle with units
	 */
	inline degMeas get_SteeringWheelAngle_meas() {
		return degMeas(this->get_SteeringWheelAngle());
	}

	/**
	 * This function is only for backwards compatibility
	 * @deprecated
	 */
	inline double get_Speed_mph()
	{
		return this->get_Speed_meas().as<units::Speed::mph>().get_value();
	}

	/**
	 * This function is only for backwards compatibility
	 * @deprecated
	 */
	inline double get_Speed_mps()
	{
		return this->get_Speed_meas().as<units::Speed::mps>().get_value();
	}

	/**
	 * This function is only for backwards compatibility
	 * @deprecated
	 */
	inline double get_Speed_kph()
	{
		return this->get_Speed_meas().as<units::Speed::kph>().get_value();
	}

	/**
	 * This function is only for backwards compatibility
	 * @deprecated
	 */
	inline void set_Speed_mph(double mph)
	{
		this->set_Speed(units::Convert<units::Speed, units::Speed::mph, mpsMeas::unit>(mph));
	}

	/**
	 * This function is only for backwards compatibility
	 * @deprecated
	 */
	inline void set_Speed_mps(double mps)
	{
		this->set_Speed(units::Convert<units::Speed, units::Speed::mps, mpsMeas::unit>(mps));
	}

	/**
	 * This function is only for backwards compatibility
	 * @deprecated
	 */
	inline void set_Speed_kph(double kph)
	{
		this->set_Speed(units::Convert<units::Speed, units::Speed::kph, mpsMeas::unit>(kph));
	}
};

} /* namespace messages */
} /* namespace tmx */


#endif /* INCLUDE_DECODEDBSMMESSAGE_H_ */
