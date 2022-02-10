#include "AMQP_Base.h"

AMQP_Base::AMQP_Base(){

}

bool AMQP_Base::Reset(){
	try{
		if (this->m_channel != NULL)
		{
			this->m_channel.reset();
		}
		//this->m_channel = Channel::Create("141.219.181.216", 5672, "lab-admin", "lab-admin");
		this->m_channel = Channel::Create(ip, port, username, password);
		this->is_channel_created = true;
	}
	catch (exception ex){
		cout << ex.what() << endl;
		this->connected = false;
		this->is_channel_created = false;
	}
	return this->is_channel_created;
}

void AMQP_Base::SetConnected(bool is_connected){  
	this->connected = is_connected;
}

void AMQP_Base::channel_reset()
{
	this->m_channel.reset();
}