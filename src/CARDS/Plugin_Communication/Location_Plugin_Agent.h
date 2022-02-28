#ifndef LOCATION_PLUGIN_AGENT_H
#define LOCATION_PLUGIN_AGENT_H

#include "Plugin_Communication_System.h"

class Location_Plugin_Agent
{
private:
	int register_time_step = -1;
	int current_time_step = -1;
	int vehicle_communication_id = -1;
	
	double speed_mps = 0.0;
	double speed_mph = 0.0;
	double acceleration_mps2 = 0.0;
	double heading = 0.0;
	double latitude = 0.0;
	double longitude = 0.0;
	double altitude = 0.0;

	Plugin_Communication_System plugin_register;
	Plugin_Communication_System plugin_d2v_communication_system;

public:
	void Location_Plugin_Agent_Initialize();
	void Location_Plugin_Agent_receive_from_digital();

	double get_speed_mps(){return this->speed_mps;};
	double get_speed_mph(){return this->speed_mph;};
	double get_acceleration_mps2(){return this->acceleration_mps2;};
	double get_latitude(){return this->latitude;};
	double get_longitude(){return this->longitude;};
	double get_altitude(){return this->altitude;};
	double get_heading(){return this->heading;};
};

#endif