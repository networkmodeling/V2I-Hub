/*
 * VehicleBasicMessage.h
 *
 *  Created on: Apr 8, 2016
 *      Author: ivp
 */

#ifndef INCLUDE__VEHICLEBASICMESSAGE_H_
#define INCLUDE__VEHICLEBASICMESSAGE_H_

#include <tmx/messages/message.hpp>
#include "MessageTypes.h"
#include "Measurement.h"
#include "VehicleParameterEnumTypes.h"

namespace tmx {
namespace messages {

/**
 * VehicleBasicMessage is the message type used to transmit information from the vehicle through TMX core.
 * It defines the message type and sub type and all data members.
 */
class VehicleBasicMessage : public tmx::message
{
	typedef Measurement<units::Angle, units::Angle::deg> degMeas;
	typedef Measurement<units::Percent, units::Percent::pct> pctMeas;
	typedef Measurement<units::Speed, units::Speed::mph> mphMeas;
	typedef Measurement<units::Acceleration, units::Acceleration::mperspers> mperspersMeas;
	typedef Measurement<units::Frequency, units::Frequency::rpm> rpmMeas;
	typedef Measurement<units::Distance, units::Distance::m> mMeas;
	typedef Measurement<units::Temperature, units::Temperature::C> CMeas;

public:
	VehicleBasicMessage() {}
	VehicleBasicMessage(const tmx::message_container_type &contents): tmx::message(contents) {}

	/// Message type for routing this message through TMX core.
	static constexpr const char* MessageType = MSGTYPE_VEHICLE_STRING;

	/// Message sub type for routing this message through TMX core.
	static constexpr const char* MessageSubType = MSGSUBTYPE_BASIC_STRING;

	/// The gear shift position.
	std_attribute(this->msg, tmx::Enum<vehicleparam::GearState>, GearPosition, vehicleparam::GearState::GearUnknown, )

	/// Indicates whether the brake is currently applied.
	std_attribute(this->msg, tmx::Enum<vehicleparam::GenericState>, Brake, vehicleparam::GenericState::Inactive, )

	/// Indicates whether the anti-lock brake system is currently applied.
	std_attribute(this->msg, tmx::Enum<vehicleparam::GenericState>, ABS, vehicleparam::GenericState::Inactive, )

	/// Indicates whether the stability control system is currently applied.
	std_attribute(this->msg, tmx::Enum<vehicleparam::GenericState>, StabilityControl, vehicleparam::GenericState::Inactive, )

	/// The turn signal position.
	std_attribute(this->msg, tmx::Enum<vehicleparam::TurnSignalState>, TurnSignalPosition, vehicleparam::TurnSignalState::SignalUnknown, )

	/// The front door status
	std_attribute(this->msg, tmx::Enum<vehicleparam::DoorState>, FrontDoors, vehicleparam::DoorState::Closed, );

	/// The rear door status
	std_attribute(this->msg, tmx::Enum<vehicleparam::DoorState>, RearDoors, vehicleparam::DoorState::Closed, );

	/// The status of the head lights
	std_attribute(this->msg, tmx::Enum<vehicleparam::GenericState>, HeadLights, vehicleparam::GenericState::Inactive, );

	/// The status of the high beam headlights, i.e. the brights
	std_attribute(this->msg, tmx::Enum<vehicleparam::GenericState>, HighBeam, vehicleparam::GenericState::Inactive, );

	/// The status of the tail lights
	std_attribute(this->msg, tmx::Enum<vehicleparam::GenericState>, TailLights, vehicleparam::GenericState::Inactive, );

	/// The status of the brake lights
	std_attribute(this->msg, tmx::Enum<vehicleparam::GenericState>, BrakeLights, vehicleparam::GenericState::Inactive, );

	/// The status of the wipers
	std_attribute(this->msg, tmx::Enum<vehicleparam::WiperState>, Wipers, vehicleparam::WiperState::WiperUnknown, );

	/// The steering wheel angle
	std_attribute(this->msg, degMeas, SteeringWheelAngle, 0.0, );

	/// The accelerator pedal position
	std_attribute(this->msg, pctMeas, AcceleratorPosition, 0.0, );

	/// The speed of the vehicle in meters per second
	std_attribute(this->msg, mphMeas, Speed, 0.0, )

	/// The acceleration of the vehicle
	std_attribute(this->msg, mperspersMeas, Acceleration, 0.0, );

	/// The speed of the left front wheelt
	std_attribute(this->msg, rpmMeas, LeftFrontWheel, 0.0, );

	/// The speed of the right front wheelt
	std_attribute(this->msg, rpmMeas, RightFrontWheel, 0.0, );

	/// The speed of the left front wheelt
	std_attribute(this->msg, rpmMeas, LeftRearWheel, 0.0, );

	/// The speed of the right front wheelt
	std_attribute(this->msg, rpmMeas, RightRearWheel, 0.0, );

	/// The length of the vehicle
	std_attribute(this->msg, mMeas, VehicleLength, 0.0, );

	/// The outside air temperature
	std_attribute(this->msg, CMeas, Temp, 0.0, );

	/**
	 * This function is only for backwards compatibility
	 * @deprecated
	 */
	inline double get_Speed_mph()
	{
		return this->get_Speed().as<units::Speed::mph>().get_value();
	}

	/**
	 * This function is only for backwards compatibility
	 * @deprecated
	 */
	inline double get_Speed_mps()
	{
		return this->get_Speed().as<units::Speed::mps>().get_value();
	}

	/**
	 * This function is only for backwards compatibility
	 * @deprecated
	 */
	inline double get_Speed_kph()
	{
		return this->get_Speed().as<units::Speed::kph>().get_value();
	}

	/**
	 * This function is only for backwards compatibility
	 * @deprecated
	 */
	inline void set_Speed_mph(double mph)
	{
		this->set_Speed(units::Convert<units::Speed, units::Speed::mps, Speed::data_type::unit>(mph));
	}

	/**
	 * This function is only for backwards compatibility
	 * @deprecated
	 */
	inline void set_Speed_mps(double mps)
	{
		this->set_Speed(units::Convert<units::Speed, units::Speed::mps, Speed::data_type::unit>(mps));
	}

	/**
	 * This function is only for backwards compatibility
	 * @deprecated
	 */
	inline void set_Speed_kph(double kph)
	{
		this->set_Speed(units::Convert<units::Speed, units::Speed::kph, Speed::data_type::unit>(kph));
	}
};

} /* namespace messages */
} /* namespace tmx */

#endif /* INCLUDE__VEHICLEBASICMESSAGE_H_ */
