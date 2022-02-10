#ifndef _AMQPBASE_H
#define _AMQPBASE_H

#include <iostream>
#include <unordered_set>
#include <SimpleAmqpClient/SimpleAmqpClient.h>

using namespace std;
using namespace AmqpClient;

class AMQP_Base{
public:
	AMQP_Base();
	~AMQP_Base(){ m_channel.reset(); };
	void SetConnected(bool is_connected);
	void channel_reset();
protected:
	string ip;
	int port;
	string username;
	string password;
	string exchange;
	Channel::ptr_t m_channel;
	bool connected = false;
	bool is_channel_created = false;
	bool Reset();
};
#endif