#ifndef PLUGIN_COMMUNICATION_SYSTEM_H
#define PLUGIN_COMMUNICATION_SYSTEM_H

#include "AMQP_Publisher.h"
#include "AMQP_Subscriber.h"
#include "util_time_operation.h"
#include <iostream>
#include <string.h>
#include "json/json.h"
#include "cfg_reader.h"

using namespace std;
using namespace utility_function;

class Plugin_Communication_System
{
private:
	int communication_id = -1;
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


#endif