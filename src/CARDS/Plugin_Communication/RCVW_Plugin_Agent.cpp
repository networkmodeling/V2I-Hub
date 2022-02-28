#include "RCVW_Plugin_Agent.h"

void RCVW_Plugin_Agent::RCVW_Plugin_Agent_Initialize()
{
	// initialize register
	
	string rabbitmq_server_ip = "141.219.181.162";
	int rabbitmq_port = 5672;
	string rabbitmq_username = "adm";
	string rabbitmq_password = "adm";
	string rabbitmq_exchange_name = "HIL_test_exchange";
	string uuid = "98-2C-BC-76-55-DE-2021-02-14-11-22-36-39";

	plugin_register.initialize(rabbitmq_server_ip, rabbitmq_username, rabbitmq_password, rabbitmq_exchange_name, uuid);

	// send register msg

	// Hancock-Houghton network	
	string line = "106666,23,28,57,78,282,544,00:00:03.0,24:00:14.0,5.5846684092186e-315,195,,SOV,,0,0,0,0,OUT_NETWORK,1,57,78,0,0,0,0,1,-1,108001,1";
	// Simple Grade Crossing network
	// string line = "106666,1,2,1,1,4,6,00:00:03.0,33:06:02.0,0,0,,SOV,,0,0,0,0,OUT_NETWORK,1,1,3,0,0,0,0,-1,-1";

	Json::Value root;
	root["Application_Type"] = "CARDS_CAV_OBU_APPLICATION";
	root["content"] = line;
	root["UUID"] = uuid;
	root["CARDS_DATA_EXCHANGE_MODE"] = "CARDS_DATA_EXCHANGE_AMQP";
	root["CARDS_MODE_OF_PHYSICAL_AGENT"] = "CARDS_VIRTUAL_WORLD";
	root["V2X_HUB_PLUGIN_TYPE"] = "LOCATION_PLUGIN_RCVW_PLUGIN";

	string sent_register_message = root.toStyledString();
	bool is_success = false;
	plugin_register.publish_message_to_queue_amqp(is_success, sent_register_message, "register");
	cout << "~~~~~ sent register message: " << sent_register_message << endl;

	// receive register ack msg

	string received_register_ack_message = plugin_register.subscribe_message(-1);
	cout << "~~~~~ received register ack message: " << received_register_ack_message << endl;

	CfgReader cfgReader;
	cfgReader.initialize(received_register_ack_message.c_str(), false);
	string vehicle_communication_id_string, register_time_step_string;
	if (cfgReader.getParameter("vehicle_communication_id", &vehicle_communication_id_string) == PARAMETER_FOUND) 
	{
		this->vehicle_communication_id = stoi(vehicle_communication_id_string);
		cout << "~~~~~ received register ack message vehicle_communication_id = " << this->vehicle_communication_id << endl;
	}
	if (cfgReader.getParameter("register_time_step", &register_time_step_string) == PARAMETER_FOUND) 
	{
		this->register_time_step = stoi(register_time_step_string);
		this->current_time_step = stoi(register_time_step_string);
		cout << "~~~~~ received register ack message register_time_step = " << this->register_time_step << endl;
	}

	// initialize d2v communication system

	string d2vw_subscriber_name = "d2vw_message."+to_string(this->vehicle_communication_id);
	plugin_d2vw_communication_system.initialize(rabbitmq_server_ip, rabbitmq_username, rabbitmq_password, rabbitmq_exchange_name, d2vw_subscriber_name);
	cout << "~~~~~ ready to receive d2vw message " << endl;

}


void RCVW_Plugin_Agent::RCVW_Plugin_Agent_receive_from_digital()
{
	// receive d2vw msg 
	cout << "~~~~~ current time step = " << current_time_step << endl;
	string received_d2vw_message = "";
	received_d2vw_message = plugin_d2vw_communication_system.subscribe_message(-1);
	if (!received_d2vw_message.empty())
	{
		cout << "~~~~~ received d2vw message = " << received_d2vw_message << endl;
	}
	else
	{
		cout << "~~~~~ failed to received d2vw message" << endl;
	}

	// ack d2vw msg, send RCVW status
	string line = "";
	line += to_string(vehicle_communication_id);
	line += ",";
	line += to_string(current_time_step);
	line += ",";
	line += rcvw_available_active ? "1," : "0,";
	line += rcvw_approach_inform_active ? "1," : "0,";
	line += rcvw_approach_warning_active ? "1," : "0,";
	line += rcvw_hri_warning_active ? "1," : "0,";
	line += rcvw_error_active ? "1" : "0";

	string sent_d2vw_ack_message = line;
	bool is_success = false;
	string v2dw_subscriber_name = "v2dw_message." + to_string(vehicle_communication_id);
	plugin_d2vw_communication_system.publish_message_to_queue_amqp(is_success, sent_d2vw_ack_message, v2dw_subscriber_name);
	cout << "~~~~~ sent d2vw ack message: " << sent_d2vw_ack_message << endl;

	// update time step
	current_time_step += 1;

}

void RCVW_Plugin_Agent::set_rcvw_status(
		bool rcvw_available_active_status,
		bool rcvw_approach_inform_active_status,
		bool rcvw_approach_warning_active_status,
		bool rcvw_hri_warning_active_status,
		bool rcvw_error_active_status)
{
	this->rcvw_available_active = rcvw_available_active_status;
	this->rcvw_approach_inform_active = rcvw_approach_inform_active_status;
	this->rcvw_approach_warning_active = rcvw_approach_warning_active_status;
	this->rcvw_hri_warning_active = rcvw_hri_warning_active_status;
	this->rcvw_error_active = rcvw_error_active_status;
}
