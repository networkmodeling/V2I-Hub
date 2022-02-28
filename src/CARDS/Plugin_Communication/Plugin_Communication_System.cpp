#include "Plugin_Communication_System.h"

void Plugin_Communication_System::initialize(string& ip, string& rab_username, string& rab_password, string& exchange_name, string& subscriber_name)
{
    this->ip = ip;
    this->rab_username = rab_username;
    this->rab_password = rab_password;
    this->exchange_name = exchange_name;
    this->subscriber_name = subscriber_name;

    try
    {
        subscriber.AMQP_initialize(this->ip, this->port, this->rab_username, this->rab_password, this->exchange_name, this->subscriber_name, true);
        subscriber.AMQP_add_routing_key(this->subscriber_name);
    }
    catch (const std::exception& ex)
	{
		cout << ex.what() << endl;
	}    
}

void Plugin_Communication_System::publish_message_to_queue_amqp(bool& is_success, string message, string queue_id)
{
	try 
    {
		AMQP_Publisher publisher;
		publisher.AMQP_intialize(this->ip, this->port, this->rab_username, this->rab_password, this->exchange_name);
		publisher.publish(queue_id, message, &is_success);
		// thread t(&AMQP_Publisher::publish, publisher, queue_id, message, &is_success);
		// t.detach();

		double start_time = get_current_cpu_time_in_milliseconds();
		while (!is_success) {
			//todo: may need to change the time out
			if (get_current_cpu_time_in_milliseconds() - start_time > 300) {
				//t.~thread();
				return;
			}
			else {
				continue;
			}
		}
	}
	catch (std::exception& ex) {
		cout << ex.what() << endl;
	}
}

string Plugin_Communication_System::subscribe_message(int time_out)
{
    string received_message = "";
    subscriber.subscribe(received_message, time_out);
    return received_message;
}