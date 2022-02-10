#include "AMQP_Publisher.h"

AMQP_Publisher::AMQP_Publisher():AMQP_Base(){

}

AMQP_Publisher::~AMQP_Publisher()
{
	if (!passive){
		//this->m_channel->DeleteExchange(this->exchange);
	}
	if (connected){
		connected = false;
	}
	//cout << "release rabbitmq publisher" << endl;
}

void AMQP_Publisher::AMQP_intialize(string ip, int port, string username, string password, string exchange)
{
	this->ip = ip;
	this->port = port;
	this->username = username;
	this->password = password;
	this->exchange = exchange;
	Reset();
	this->SetConnected(true);
}

void AMQP_Publisher::AMQP_initialize_for_cloud(string ip, int port, string username, string password, string exchange)
{
	this->ip = ip;
	this->port = port;
	this->username = username;
	this->password = password;
	this->exchange = exchange;
	this->passive = false;
	Reset();
	try{
		this->m_channel->DeclareExchange(this->exchange, "topic", this->passive, false, !this->passive);
		cout << "success to create to exchange: " << this->exchange << endl;
		this->SetConnected(true);
	}
	catch (exception & ex){
		this->SetConnected(false);
		cout << "fail to create exchange: " << this->exchange << "\t" << ex.what() << endl;
	}
}

void AMQP_Publisher::AMQP_initialize_for_vehicle(string ip, int port, string username, string password, string exchange)
{
	this->ip = ip;
	this->port = port;
	this->username = username;
	this->password = password;
	this->exchange = exchange;
	this->passive = true;
	Reset();
	try{
		this->m_channel->DeclareExchange(this->exchange, "topic", this->passive, false, !this->passive);
		cout << "success to connect to exchange: " << this->exchange << endl;
		this->SetConnected(true);
	}
	catch (ChannelException const & ex){
		this->SetConnected(false);
		cout << "'" << this->exchange << "' does not exist, please check: 1.if the server program is running; 2.if the 'exchange' has been configure rightly in configure.txt " << endl;
		cout << "the error is caused by " << ex.what() << endl;
	}
	catch (exception & ex){
		this->SetConnected(false);
		this->connected = false;
		cout << "fail to create channel to communicate with rabbitmq, please check the network connection \nthe ip address is: " << ip << " and the port is: " << port << endl;
		cout << "the error is caused by " << ex.what() << endl;
	}
}

void AMQP_Publisher::publish(string receiver, string message, bool * is_success){
	/*BasicMessage::ptr_t outgoing_message = BasicMessage::Create();
	outgoing_message->Body(message);
	this->m_channel->BasicPublish(this->exchange, receiver, outgoing_message);*/
	try {
		BasicMessage::ptr_t outgoing_message = BasicMessage::Create();
		outgoing_message->Body(message);
		this->m_channel->BasicPublish(this->exchange, receiver, outgoing_message);
		*is_success = true;
	}
	catch (std::exception const &ex) {
#ifdef _DEBUG
		cout << ex.what() << endl;
#endif // _DEBUG
		cout << "fail to publish message: " << message << endl;
	}
}

void AMQP_Publisher::reset_publish()
{
	if (this->Reset())
	{
		if (this->passive)
		{
			this->AMQP_initialize_for_cloud(ip, port, username, password, exchange);
		}
		else
		{
			this->AMQP_initialize_for_vehicle(ip, port, username, password, exchange);
		}
	}
}

bool AMQP_Publisher::is_connected(){
	return this->connected;
}