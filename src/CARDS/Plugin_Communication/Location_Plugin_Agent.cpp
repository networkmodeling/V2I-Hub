#include "Location_Plugin_Agent.h"

void Location_Plugin_Agent::Location_Plugin_Agent_Initialize()
{
	// initialize register
	
	string rabbitmq_server_ip = "141.219.181.162";
	int rabbitmq_port = 5672;
	string rabbitmq_username = "adm";
	string rabbitmq_password = "adm";
	string rabbitmq_exchange_name = "HIL_test_exchange";
	string uuid = "98-2C-BC-76-55-DE-2021-02-14-11-22-36-40";

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
	root["V2X_HUB_PLUGIN_TYPE"] = "LOCATION_PLUGIN";

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

	string d2v_subscriber_name = "d2v_message."+to_string(this->vehicle_communication_id);
	plugin_d2v_communication_system.initialize(rabbitmq_server_ip, rabbitmq_username, rabbitmq_password, rabbitmq_exchange_name, d2v_subscriber_name);
	cout << "~~~~~ ready to receive d2v message " << endl;

}

void Location_Plugin_Agent::Location_Plugin_Agent_receive_from_digital()
{
	// receive d2v msg once
	cout << "~~~~~ current time step = " << current_time_step << endl;
	string received_d2v_message = "";
	received_d2v_message = plugin_d2v_communication_system.subscribe_message(-1);
	if (!received_d2v_message.empty())
	{
		cout << "~~~~~ received d2v message = " << received_d2v_message << endl;
	}
	else
	{
		cout << "~~~~~ failed to received d2v message" << endl;
	}

	// parse d2v msg
	string delimiter = ",";
	vector<string> parameter_value_strings;
	string_split(parameter_value_strings, received_d2v_message, delimiter);
	this->speed_mps = stod(parameter_value_strings[7]);
	this->speed_mph = this->speed_mps * 2.23694;
	this->acceleration_mps2 = stod(parameter_value_strings[8]);
	this->longitude = stod(parameter_value_strings[9]);
	this->latitude = stod(parameter_value_strings[10]);
	this->altitude = stod(parameter_value_strings[11]);
	convert_matrix_to_mecator(this->longitude, this->latitude, 0.0, 0.0, -9848988.004, 5955648.631);  // Hancock-Houghton network
	// convert_matrix_to_mecator(this->longitude, this->latitude, 0.0, 0.0, -337.134, 94.212);  // Simple Grade Crossing network
	convert_mecator_to_WGS84(this->longitude, this->latitude);

	cout << "~~~~~ current vehicle status is: speed_mps = " << speed_mps
	<< ", speed_mph = " << speed_mph
	<< ", acceleration_mps2 = " << acceleration_mps2
	<<", latitude = " << latitude
	<<", longitude = " << longitude
	<<", altitude = " << altitude << endl;

	// ack d2v msg once
	string sent_d2v_ack_message = "106666,0,0,5,0,0,0,1.4931647780876,0";
	bool is_success = false;
	string v2d_subscriber_name = "v2d_message." + to_string(vehicle_communication_id);
	plugin_d2v_communication_system.publish_message_to_queue_amqp(is_success, sent_d2v_ack_message, v2d_subscriber_name);
	cout << "~~~~~ sent d2v ack message: " << sent_d2v_ack_message << endl;

	// update time step
	current_time_step += 1;

}