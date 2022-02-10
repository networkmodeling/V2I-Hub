#ifndef AMQP_PUBLISHER_H
#define AMQP_PUBLISHER_H
#include "AMQP_Base.h"

class AMQP_Publisher : public AMQP_Base{
public:
	AMQP_Publisher();
	~AMQP_Publisher();
	void AMQP_intialize(string ip, int port, string username, string password, string exchange);
	void AMQP_initialize_for_cloud(string ip, int port, string username, string password, string exchange);
	void AMQP_initialize_for_vehicle(string ip, int port, string username, string password, string exchange);
	
	void publish(string receiver, string message, bool * is_true);
	void reset_publish();
	bool is_connected();
private:
	bool passive;
};

#endif // !AMQP_PUBLISHER_H