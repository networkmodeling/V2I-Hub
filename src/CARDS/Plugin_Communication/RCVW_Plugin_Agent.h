#ifndef RCVW_PLUGIN_AGENT_H
#define RCVW_PLUGIN_AGENT_H

#include "Plugin_Communication_System.h"

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