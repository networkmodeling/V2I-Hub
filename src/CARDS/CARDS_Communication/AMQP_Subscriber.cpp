#include "AMQP_Subscriber.h"

//defintion of AMQP_Subscriber_Base
AMQP_Subscriber_Base::AMQP_Subscriber_Base() :AMQP_Base()
{
}

AMQP_Subscriber_Base::~AMQP_Subscriber_Base()
{
	if(this->m_channel !=NULL)
		this->m_channel->DeleteQueue(this->queue_id);
}

bool AMQP_Subscriber_Base::is_connected()
{
	return this->connected;
}

void AMQP_Subscriber_Base::AMQP_add_routing_key(string routing_key)
{
	try{
		this->routing_keys.insert(routing_key);
		this->m_channel->BindQueue(queue_id, exchange, routing_key);
		cout << "bind routing key: " << routing_key << " to consumer" << endl;
		//cout << "exchange name:" << this->exchange << endl;
	}
	catch (std::exception const & ex)
	{
		this->connected = false;
		cout << "fail to set subscriber to receive message, please check the network connection \nthe ip address is: " << ip << " and the port is: " << port << endl;
		cout << ex.what() << endl;
	}
}


void AMQP_Subscriber_Base::AMQP_base_initialize(string ip, int port, string username, string password, string exchange, string queueid, bool is_cloud)
{
	this->ip = ip;
	this->port = port;
	this->username = username;
	this->password = password;
	this->exchange = exchange;
	this->queue_id = queueid;
	this->is_cloud = is_cloud;
	boost::uint32_t message_count = 0;
	boost::uint32_t consumer_count = 0;
	Reset();
	try{
		//this->m_channel->DeleteQueue(queueid);
		this->m_channel->DeclareExchange(this->exchange, "topic", !is_cloud, false,false);
		//this->m_channel->DeclareQueue(queueid, false, true, true, true);
		queue = this->m_channel->DeclareQueueWithCounts(queueid, message_count, consumer_count, false, true, true, true);
		//cout << "message count " << message_count << endl;
		//AMQP_add_routing_key(queueid);
	}
	catch (exception ex)
	{
		cout << ex.what() << endl;
	}
}

int AMQP_Subscriber_Get::get_number_of_messages()
{
	boost::uint32_t message_count = 0;
	boost::uint32_t consumer_count = 0;
	try 
	{
		Table properties;
		properties.insert(TableEntry(TableKey("x-max-length"), TableValue(this->max_queue_length)));
		string queue = this->m_channel->DeclareQueueWithCounts(queue_id, message_count, consumer_count, false, false,true, true, properties);
	}
	catch (std::exception const& ex)
	{
		this->SetConnected(false);
		cout << "fail to set subscriber to receive message, please check the network connection \nthe ip address is: " << ip << " and the port is: " << port << endl;
		cout << ex.what() << endl;
	}
	return message_count;
}

//defintion of AMQP_Subscriber_Consume
AMQP_Subscriber_Consume::AMQP_Subscriber_Consume() :AMQP_Subscriber_Base()
{
}

AMQP_Subscriber_Consume::~AMQP_Subscriber_Consume()
{
}

void AMQP_Subscriber_Consume::AMQP_initialize(string & ip, int & port, string & username, string & password, string & exchange, string & queueid, bool is_cloud)
{
	try{
		
		string queue = queueid;
		this->AMQP_base_initialize(ip, port, username, password, exchange, queue, is_cloud);
		//queue = this->m_channel->DeclareQueueWithCounts(queueid, message_count, consumer_count, false, true, true, true);
		this->m_consumer = m_channel->BasicConsume(queueid, "", true);
		for (unordered_set<string>::iterator key_iter = routing_keys.begin(); key_iter != routing_keys.end(); key_iter++)
		{
			string key_id = *key_iter;
			routing_keys.insert(key_id);
		}

		this->SetConnected(true);
	}
	catch (std::exception const & ex)
	{
		this->SetConnected(false);
		cout << "fail to set subscriber to receive message, please check the network connection \nthe ip address is: " << ip << " and the port is: " << port << endl;
		cout << ex.what() << endl;
	}
}

void AMQP_Subscriber_Consume::subscribe(string & message , int timeout)
{
	message = "";
	Envelope::ptr_t envelope;
	if (this->m_channel->BasicConsumeMessage(this->m_consumer, envelope, timeout)){
		message = envelope->Message()->Body();
		//cout << "received message:" << message<< endl;
	}
	//return message;
}

bool AMQP_Subscriber_Consume::ResetSubscriber(){
	if (this->Reset()){
		AMQP_initialize(ip, port, username, password, exchange, queue_id, this->is_cloud);
	}
	return this->is_connected();
}

//definition of AMQP_Subsriber_Get
AMQP_Subscriber_Get::AMQP_Subscriber_Get()
{
}

AMQP_Subscriber_Get::~AMQP_Subscriber_Get()
{
}

void AMQP_Subscriber_Get::AMQP_initialize(string ip, int port, string username, string password, string exchange, string queueid, int max_length, bool is_cloud)
{
	try{
		this->AMQP_base_initialize(ip, port, username, password, exchange, queueid, is_cloud);
		this->m_channel->DeleteQueue(queueid);
		this->max_queue_length = max_length;
		get_number_of_messages();
		this->m_channel->BindQueue(queueid, this->exchange, queueid);
		this->SetConnected(true);
	}
	catch (std::exception const & ex)
	{
		this->SetConnected(false);
		cout << "fail to set subscriber to receive message, please check the network connection \nthe ip address is: " << ip << " and the port is: " << port << endl;
		cout << ex.what() << endl;
	}
}


std::string AMQP_Subscriber_Get::subscribe()
{
	string message = "";
	Envelope::ptr_t envelope;
	if (this->m_channel->BasicGet(envelope, this->queue_id)){
		message = envelope->Message()->Body();
	}
	return message;
}

bool AMQP_Subscriber_Get::ResetSubscriber(){
	if (this->Reset()){
		AMQP_initialize(ip, port, username, password, exchange, queue_id, max_queue_length, this->is_cloud);
	}
	return this->is_connected();
}