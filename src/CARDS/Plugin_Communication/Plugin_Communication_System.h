#ifndef PLUGIN_COMMUNICATION_SYSTEM_H
#define PLUGIN_COMMUNICATION_SYSTEM_H

#include "AMQP_Publisher.h"
#include "AMQP_Subscriber.h"
#include "util_time_operation.h"
#include "util_geo_operation.h"
#include <iostream>
#include <string.h>
#include "json/json.h"
#include "cfg_reader.h"

using namespace std;
using namespace utility_function;

class Plugin_Communication_System
{
private:
	// int communication_id = -1;
    string ip = "localhost";
	int port = 5672;
	string rab_username = "";
	string rab_password = "";
	string exchange_name = "";
	string subscriber_name = "";

public:
    AMQP_Subscriber_Consume subscriber;
	AMQP_Subscriber_Get subscriber_get;

    void initialize(string& ip, string& rab_username, string& rab_password, string& exchange_name, string& subscriber_name);
    void publish_message_to_queue_amqp(bool& is_success, string message, string queue_id);
    string subscribe_message(int time_out);
	string get_subscriber_name(){return this->subscriber_name;};

};

class HRI_Status_Plugin_Agent
{
private: 
	int register_time_step = -1;
	int current_time_step = -1;
	int RSU_communication_id = -1;
	
	bool preenption_signal_status = false;

	Plugin_Communication_System plugin_register;
	Plugin_Communication_System plugin_d2i_communication_system;

public:
	void HRI_Status_Plugin_Agent_Initialize();
	void HRI_Status_Plugin_Agent_receive_from_digital();
	bool get_preemption_signal_status(){return this->preenption_signal_status;};

};

class HRI_Predicted_SPaT_Plugin_Agent
{
private: 
	int register_time_step = -1;
	int current_time_step = -1;
	int RSU_communication_id = -1;
	
	bool preenption_signal_status = false;
	int minEndTime = -1;
	int maxEndTime = -1;
	int currentTime = -1;

	Plugin_Communication_System plugin_register;
	Plugin_Communication_System plugin_d2i_communication_system;

public:
	void HRI_Predicted_SPaT_Plugin_Agent_Initialize();
	void HRI_Predicted_SPaT_Plugin_Agent_receive_from_digital();
	bool get_preemption_signal_status(){return this->preenption_signal_status;};
	int get_minEndTime(){return this->minEndTime;};
	int get_maxEndTime(){return this->maxEndTime;};
	int get_currentTime(){return this->currentTime;};

};


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

class RCVW_Plugin_Agent
{
private:
	int register_time_step = -1;
	int current_time_step = -1;
	int vehicle_communication_id = -1;
	
	bool rcvw_available_active = false;
	bool rcvw_approach_inform_active = false;
	bool rcvw_approach_warning_active = false;
	bool rcvw_hri_warning_active = false;
	bool rcvw_error_active = false;

	Plugin_Communication_System plugin_register;
	Plugin_Communication_System plugin_d2vw_communication_system;

public:
	void RCVW_Plugin_Agent_Initialize();
	void RCVW_Plugin_Agent_receive_from_digital();

	void set_rcvw_status(
		bool rcvw_available_active_status,
		bool rcvw_approach_inform_active_status,
		bool rcvw_approach_warning_active_status,
		bool rcvw_hri_warning_active_status,
		bool rcvw_error_active_status);

};


#endif