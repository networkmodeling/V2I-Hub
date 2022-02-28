#ifndef HRI_PREDICTED_SPAT_PLUGIN_AGENT_H
#define HRI_PREDICTED_SPAT_PLUGIN_AGENT_H

#include "Plugin_Communication_System.h"
#include "Plugin_Enum.h"

class HRI_Predicted_SPaT_Plugin_Agent
{
private: 
	int register_time_step = -1;
	int current_time_step = -1;
	int RSU_communication_id = -1;
	
	CrossingStatus current_phase = UNAVAILABLE;
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

#endif