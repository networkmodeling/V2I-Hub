#ifndef HRI_STATUS_PLUGIN_AGENT_H
#define HRI_STATUS_PLUGIN_AGENT_H

#include "Plugin_Communication_System.h"

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