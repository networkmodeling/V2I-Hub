/*
 * LocationMessage.h
 *
 *  Created on: Apr 21, 2016
 *      Author: ivp
 */

#ifndef INCLUDE_LOCATIONMESSAGE_H_
#define INCLUDE_LOCATIONMESSAGE_H_

#include <tmx/messages/message.hpp>
#include "MessageTypes.h"
#include "LocationMessageEnumTypes.h"

#include "Measurement.h"

namespace tmx {
namespace messages {

/**
 * LocationMessage is the message type used to send information messages about plugin status/activities.
 * It defines the message type and sub type and all data members.
 */
class LocationMessage : public tmx::message
{
	typedef Measurement<units::Angle, units::Angle::deg> degMeas;
	typedef Measurement<units::Distance, units::Distance::m> mMeas;
	typedef Measurement<units::Speed, units::Speed::mps> mpsMeas;

public:
	LocationMessage() {}
	LocationMessage(const tmx::message_container_type &contents): tmx::message(contents) {}
	LocationMessage(std::string id, location::SignalQualityTypes signalQuality, std::string sentenceIdentifier, std::string time,
			double latitude, double longitude, location::FixTypes fixQuality, int numSatellites, double horizontalDOP, double speed, double heading) {
		set_Id(id);
		set_SignalQuality(signalQuality);
		set_SentenceIdentifier(sentenceIdentifier);
		set_Time(time);
		set_Latitude(latitude);
		set_Longitude(longitude);
		set_FixQuality(fixQuality);
		set_NumSatellites(numSatellites);
		set_HorizontalDOP(horizontalDOP);
		set_Speed_mps(speed);
		set_Heading(heading);
	}

	/// Message type for routing this message through TMX core.
	static constexpr const char* MessageType = MSGTYPE_DECODED_STRING;

	/// Message sub type for routing this message through TMX core.
	static constexpr const char* MessageSubType = MSGSUBTYPE_LOCATION_STRING;

	std_attribute(this->msg, std::string, Id, "", )
	std_attribute(this->msg, location::SignalQualityTypes, SignalQuality, location::SignalQualityTypes::Invalid, )
	/**
		 * $GPGGA Global Positioning System Fix Data.Time, position and fix related data for a GPS receiver.
		 */
	std_attribute(this->msg, std::string, SentenceIdentifier, "", )
		/**
		 * hhmmss.ss = UTC of position. (ex: 170834	        is  17:08:34 Z)
		 */
	std_attribute(this->msg, std::string, Time, "", )
		/**
		 * llll.ll = latitude of position (ex: 4124.8963, N        is 	41d 24.8963' N or 41d 24' 54" N)
		 * 	a = N or S
		 */
	std_attribute(this->msg, double, Latitude, 0.0, )
		/**
		 * 	yyyyy.yy = Longitude of position (ex: 08151.6838, W        is 81d 51.6838' W or 81d 51' 41" W)
		 * a = E or W
		 */
	std_attribute(this->msg, double, Longitude, 0.0, )
		/**
		 * 	x = GPS Quality indicator (0=no fix, 1=GPS fix, 2=Dif. GPS fix)
		 */
	std_attribute(this->msg, location::FixTypes, FixQuality, location::FixTypes::Unknown, )
		/**
		 * 	xx = number of satellites in use (ex: 	05	is 5 Satellites are in view)
		 */
	std_attribute(this->msg, int, NumSatellites, 0, )
		/**
		 * 	x.x = horizontal dilution of precision (ex: 1.5	is Relative accuracy of horizontal position)
		 */
	std_attribute(this->msg, double, HorizontalDOP, 0, )
		/**
		 * 	x.x = Antenna altitude above mean-sea-level (ex: 280.2, M	is   280.2 meters above mean sea level)
	M = units of antenna altitude, meters
		 */
	std_attribute(this->msg, mMeas, Altitude, 0.0, )
		/**
		 * x.x = Geoidal separation  - Height of geoid above WGS84 ellipsoid.  (ex: -34.0, M	is   -34.0 meters)
	M = units of geoidal separation, meters
		 */
		//std_attribute(this->msg,double, GeoidalSeparation, 0, )
		/**
		 * Time since last DGPS update.
		 * x.x = Age of Differential GPS data (seconds)
		 */
		//std_attribute(this->msg,int, SecSinceLastUpdate, 0, )
		/**
		 * DGPS reference station id
		 * xxxx = Differential reference station ID
		 */
		//std_attribute(this->msg,std:string, RefStationId, 0, )
		/**
		 * Checksum. Used by program to check for transmission errors.
		 */
		//std_attribute(this->msg,int, Checksum, 0, )

		/**
		 * x.x,K = Speed, m/s
		 *  (ex: 010.2,K      Ground speed, meters per second)
		 */
	std_attribute(this->msg, mpsMeas, Speed, 0.0, )

		/**
		 * Heading in degrees.
		 */
	std_attribute(this->msg, degMeas, Heading, 0.0, )


		//eg2. $--GGA,hhmmss.ss,llll.ll,a,yyyyy.yy,a,x,xx,x.x,x.x,M,x.x,M,x.x,xxxx

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

#endif /* INCLUDE_LOCATIONMESSAGE_H_ */
