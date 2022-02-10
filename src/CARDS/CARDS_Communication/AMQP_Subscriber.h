#ifndef AMQP_SUBSCRIBER_H
#define AMQP_SUBSCRIBER_H
#include "AMQP_Base.h"

class AMQP_Subscriber_Base :public AMQP_Base{
public:
	AMQP_Subscriber_Base();
	~AMQP_Subscriber_Base();
	bool is_connected();
	void AMQP_add_routing_key(string routing_key);
	virtual bool ResetSubscriber() = 0;
	
protected:
	void AMQP_base_initialize(string ip, int port, string username, string password, string exchange, string queueid, bool is_cloud);
	
	string queue_id;
	unordered_set<string> routing_keys;
	bool is_cloud;
	string consumer;
	string queue;
};

class AMQP_Subscriber_Consume :public AMQP_Subscriber_Base{
public:
	AMQP_Subscriber_Consume();
	~AMQP_Subscriber_Consume();
	bool is_cloud;
	void AMQP_initialize(string & ip, int & port, string & username, string & password, string & exchange, string & queueid, bool is_cloud);
	void subscribe(string & message, int timeout = 500);
	bool ResetSubscriber() override;
private:
	string m_consumer;
};

class AMQP_Subscriber_Get :public AMQP_Subscriber_Base{
public:
	AMQP_Subscriber_Get();
	~AMQP_Subscriber_Get();
	
	string subscribe();
	bool ResetSubscriber() override;
	int get_number_of_messages();
	void AMQP_initialize(string ip, int port, string username, string password, string exchange, string queueid, int max_length, bool is_cloud);
private:
	int max_queue_length;
	
};
#endif