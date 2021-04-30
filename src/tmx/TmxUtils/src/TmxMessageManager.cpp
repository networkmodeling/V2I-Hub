/*
 * @file TmxMessageManager.cpp
 *
 *  Created on: Oct 31, 2017
 *      @author: Gregory M. Baumgardner
 */

#include "TmxMessageManager.h"

#include "Clock.h"
#include "LockFreeThread.h"
#include "ThreadGroup.h"

#include <condition_variable>
#include <memory>
#include <tmx/j2735_messages/J2735MessageFactory.hpp>

namespace tmx {
namespace utils {

struct MessageStruct {
	uint8_t groupId;
	uint8_t uniqId;
	uint64_t timestamp ;
	char * encoding;
	byte_t * msgBytes;
	uint64_t msgLen;
	TmxMessageManager *mgr;
};

class RxThread: public LockFreeThread<MessageStruct> {
public:
	RxThread() {}
	~RxThread() {}

	bool accept(const MessageStruct &in);
	bool push(const MessageStruct &in);
	void wait();
	void notify();
	void Join();
protected:
	void doWork(MessageStruct &msg);
	void idle();
private:
	std::condition_variable cv;
	std::mutex _cvLock;
	std::unique_lock<mutex> lock { _cvLock };
};

static std::atomic<uint16_t> overflow {DEFAULT_OVERFLOW_CAPACITY};

// The workerThreads
static ThreadGroup workerThreads;
static ThreadGroupAssignment<uint8_t> threadAssign { workerThreads };

// The output thread
class OutputThread: public ThreadWorker {
public:
	static OutputThread &instance() {
		static OutputThread _instance;
		return _instance;
	}

	void wait();
	void notify();

	TmxMessageManager *manager = NULL;
protected:
	void DoWork();
private:
	std::condition_variable cv;
	std::mutex _cvLock;
	std::unique_lock<mutex> lock { _cvLock };
};

std::mutex _threadLock;

// This static factory initializes the map data for future use
static tmx::messages::J2735MessageFactory factory;

bool IsByteHexEncoded(const std::string &encoding)
{
	if (encoding.empty()) return false;

	// All must end in hexstring
	static const std::string hexstring("hexstring");

	return (encoding.find(hexstring) == encoding.length() - hexstring.size());
}

void RxThread::Join() {
	this->notify();
	ThreadWorker::Join();
}

void RxThread::wait() {
	FILE_LOG(logDEBUG3) << this->Id() << ": Waiting for next item in queue: queue size=" << this->inQueueSize();

	// Wait until an item appears in the queue
	this->cv.wait(this->lock, std::bind(&RxThread::inQueueSize, this));
	FILE_LOG(logDEBUG3) << this->Id() << ": Awake: queue size=" << this->inQueueSize();
}

void RxThread::notify() {
	this->cv.notify_one();
}

bool RxThread::accept(const MessageStruct &in) {
	if (in.mgr) {
		return in.mgr->Accept(in.groupId, in.uniqId);
	}

	return true;
}

/**
 * The incoming message handler will take the incoming message, construct
 * a routeable message from it, if one does not already exist, then invoke
 * the appropriate handler for that message.  The handlers must have been
 * registed already by the plugin.
 */
void RxThread::doWork(MessageStruct &msg) {
	std::unique_ptr<tmx::routeable_message> routeableMsg;

	tmx::byte_stream bytes;

	string enc;
	if (msg.encoding) {
		enc.assign(msg.encoding);

		FILE_LOG(logDEBUG4) << "RxThread encoding ptr is '" << (uint64_t)msg.encoding << "'";
		// This was a source of leaking memory, so delete here.
		free(msg.encoding);
		msg.encoding = NULL;
	}

	// Default null encoding case is a basic string
	if (enc.empty())
		enc = messages::api::ENCODING_STRING_STRING;

	if (msg.msgBytes && msg.msgLen > 0) {
		bytes.resize(msg.msgLen);
		memcpy(bytes.data(), msg.msgBytes, msg.msgLen);
		FILE_LOG(logDEBUG4) << "RxThread msgBytes ptr is '" << (uint64_t)msg.msgBytes << "'";
	}

	free(msg.msgBytes);
	msg.msgBytes = NULL;

	try {
		if (bytes.size()) {
			if (IsByteHexEncoded(enc)) {
				if (enc != messages::api::ENCODING_BYTEARRAY_STRING) {
					// New factory needed to avoid race conditions
					tmx::messages::J2735MessageFactory myFactory;

					FILE_LOG(logDEBUG4) << this->Id() << ": Decoding from bytes " << bytes;

					// Bytes are encoded.  First try to convert to a J2735 message
					routeableMsg.reset(myFactory.NewMessage(bytes));

					if (!routeableMsg)
						FILE_LOG(logDEBUG4) << this->Id() << ": Not a J2735 message: " << myFactory.get_event();
				}

				if (!routeableMsg) {
					// Set the bytes directly as unknown type
					routeableMsg.reset(new routeable_message());
					routeableMsg->set_payload_bytes(bytes);
					routeableMsg->set_encoding(enc);
				}
			} else {
				// Just use a regular string
				std::string str((const char *)bytes.data(), bytes.size());
				routeableMsg.reset(new tmx::routeable_message());
				routeableMsg->set_encoding(enc);
				if (strncmp("json", enc.c_str(), 4) == 0) {
					routeableMsg->set_contents(str);
					routeableMsg->reinit();

					FILE_LOG(logDEBUG4) << this->Id() << ": Decoding JSON " << *routeableMsg;

					// If the payload attribute is missing, then this was probably
					// a JSON payload.
					if (routeableMsg->get_payload_str().empty()) {
						FILE_LOG(logDEBUG4) << this->Id() << ": Routeable message not found, assuming payload " << *routeableMsg;
						routeableMsg.reset(new tmx::routeable_message());

						message jsonMsg(str);
						routeableMsg->set_payload(jsonMsg);
						routeableMsg->reinit();
					}
				} else {
					routeableMsg->set_payload(str);
					routeableMsg->set_encoding(enc);
				}
			}
		}
	} catch (exception &ex) {
		FILE_LOG(logERROR) << this->Id() << ": Failed to create message from incoming bytes: " << msg.msgBytes << ": " << ex.what();
	}

	// Invoke the handler
	if (routeableMsg) {
		if (msg.timestamp > 0)
			routeableMsg->set_timestamp(msg.timestamp);

		if (msg.mgr)
			msg.mgr->OnMessageReceived(*routeableMsg);

		routeableMsg.reset();
	}

	// After other messages are sent out, then push a message to clean up for this thread assignment
	msg.msgBytes = NULL;
	msg.msgLen = 0;
	if (this->push_out(msg)) {
		OutputThread::instance().notify();
	} else {
		FILE_LOG(logWARNING) << "Cleanup message for " << msg.groupId << "/" << msg.uniqId << " lost when push failed";
	}

	this_thread::yield();
}

void RxThread::idle() {
	this_thread::yield();
	wait();
}

bool RxThread::push(const MessageStruct &in) {
	if (!this->IsRunning()) {
		FILE_LOG(logERROR) << "Thread " << this->Id() << " has unexpectedly shutdown";
		return false;
	}

	bool success = LockFreeThread::push(in);
	if (success) {
		FILE_LOG(logDEBUG3) << "Notifying thread " << this->Id() << " of new IVP message";
		this->notify();
	}

	return success;
}

bool available_messages() {
	size_t numThreads = OutputThread::instance().manager->NumThreads();
	for (size_t i = 0; i < numThreads; i++)
	{
		RxThread *t = dynamic_cast<RxThread *>(workerThreads[i]);
		if (t && t->outQueueSize()) return true;
	}

	return false;
}

void OutputThread::wait() {
	this->cv.wait(lock, std::bind(&available_messages));
}

void OutputThread::notify() {
	this->cv.notify_one();
}

/**
 * The outgoing thread will pop the messages coming off of the outgoing
 * queues of each worker thread and broadcast any message that was created.
 */
void OutputThread::DoWork() {
	MessageStruct msg;

	while (IsRunning()) {
		this->wait();

		// Loop over each thread in the thread group
		size_t numThreads = OutputThread::instance().manager->NumThreads();
		for (size_t i = 0; i < numThreads; i++) {
			RxThread *t = dynamic_cast<RxThread *>(workerThreads[i]);
			if (t && t->pop(msg)) {
				if (msg.mgr) {
					if (msg.msgBytes && msg.msgLen > 0) {
						string msgStr { (const char *)msg.msgBytes, msg.msgLen };
						routeable_message rMsg { msgStr };
						rMsg.reinit();

						msg.mgr->OutgoingMessage(rMsg, true);
					}

					msg.mgr->Cleanup(msg.groupId, msg.uniqId);
				}

				if (msg.encoding) {
					FILE_LOG(logDEBUG4) << "OutputThread encoding ptr is '" << (uint64_t)msg.encoding << "'";
					free(msg.encoding);
					msg.encoding = NULL;
				}

				if (msg.msgBytes) {
					FILE_LOG(logDEBUG4) << "OutputThread msgBytes ptr is '" << (uint64_t)msg.msgBytes << "'";
					free(msg.msgBytes);
					msg.msgBytes = NULL;
				}
			}
		}

	}
}

TmxMessageManager::TmxMessageManager(std::string name):
		PluginClient(name) {
}

TmxMessageManager::~TmxMessageManager() {
	this->Stop();
}

size_t TmxMessageManager::NumThreads() {
	// This is ok because the size never shrinks
	return _numThreads;
}

bool TmxMessageManager::Accept(tmx::byte_t groupId, tmx::byte_t uniqId) {
	static std::atomic<bool> warn {false};
	uint16_t currentOverflow = overflow;

	// Check the current size of this thread
	RxThread *thread = NULL;
	int id = workerThreads.this_thread();
	if (id >= 0)
		thread = dynamic_cast<RxThread *>(workerThreads[id]);

	if (thread == NULL) return true;

	FILE_LOG(logDEBUG4) << "Current overflow value is " << currentOverflow;
	if (currentOverflow > 0 && thread->inQueueSize() > currentOverflow)
	{
		if (!warn)
		{
			FILE_LOG(logWARNING) << "Dropping messages due to incoming queue size above overflow size of "
					<< currentOverflow;
			warn = true;
		}

		// We are dropping incoming messages from the front of the queue in order to get to more relevant ones
		return false;
	}

	// Warn again the next time the queue gets too full
	warn = false;
	return true;
}

void TmxMessageManager::Cleanup(tmx::byte_t groupId, tmx::byte_t uniqId) {
	PLOG(logDEBUG4) << "Unassigning " << (int)groupId << ":" << (int)uniqId;
	threadAssign.unassign(groupId, uniqId);
}

void TmxMessageManager::IncomingMessage(const IvpMessage *msg, byte_t groupId, byte_t uniqId, uint64_t timestamp) {
	if (!msg) return;

	IvpMessage *ivpMsg = const_cast<IvpMessage *>(msg);		// The API functions do not use a const pointer
	char *jsonStr = ivpMsg_createJsonString(ivpMsg, IvpMsg_FormatOptions_none);
	string json { jsonStr };
	free(jsonStr);

	this->IncomingMessage(json, messages::api::ENCODING_JSON_STRING, groupId, uniqId, timestamp);
}

void TmxMessageManager::IncomingMessage(const tmx::routeable_message &msg, byte_t groupId, byte_t uniqId, uint64_t timestamp) {
	// Only want the actual underlying IvpMessage
	this->IncomingMessage(msg.get_message(), groupId, uniqId, timestamp);
}

void TmxMessageManager::IncomingMessage(const tmx::byte_t *bytes, size_t size, const char *encoding, byte_t groupId, byte_t uniqId, uint64_t timestamp) {
	// It was tempting to pass  the C++ object pointer to the thread,
	// but a big part of the purpose behind this class is throughput.
	// Constructing the C++ routeable_message object requires too much
	// overhead due to the attribute container.  Therefore, it is best
	// to pass the byte representation of the message.
	//	The encoding determines how to interpret the bytes.
	//	This could be:
	// 	(1) An ASN.1 encoded message, e.g. "asn.1-ber/hexstring" or "asn.1-uper/hexstring" (the default)
	//		If this is a J2735 message, the type/subtype is determined upon receipt
	//	(2) A set of raw, unencoded bytes, e.g. "bytearray/hexstring"
	//		The type/subtype of the message is Unknown
	//	(3) A literal string message, e.g. "string". The default for a NULL encoding
	//		The type/subtype of the message is Unknown
	//	(4)	An XML string message, e.g. "xmlstring"
	//		The type/subtype of the message is Unknown
	//	(5)	A JSON string message, e.g. "json". Should be used in common cases to pass type/subtype
	//		The type/subtype may be embedded in the JSON.


	if (!bytes)
		return;

	MessageStruct in;
	in.groupId = groupId;
	in.uniqId = uniqId;
	in.timestamp = timestamp;
	in.encoding = (encoding == NULL ? NULL : strdup(encoding));
	in.mgr = this;

	// Copy the bytes encoded as a hex-encoded string
	in.msgLen = size;
	in.msgBytes = (tmx::byte_t *) calloc(in.msgLen, sizeof(tmx::byte_t));
	if (in.msgBytes)
		memcpy(in.msgBytes, bytes, in.msgLen);

	PLOG(logDEBUG4) << "Incoming encoding ptr is '" << (uint64_t)in.encoding << "'";
	PLOG(logDEBUG4) << "Incoming msgBytes ptr is '" << (uint64_t)in.msgBytes << "'";
	RxThread *thread = NULL;

	tmx::byte_stream copy { bytes, bytes + size };

	do
	{

		int id = threadAssign.assign(groupId, uniqId);
		if (id < 0)
			return;

		PLOG(logDEBUG4) << "Assigning message bytes " << copy << " as " << (int)groupId << ":" << (int)uniqId << " to thread " << id;

		thread = dynamic_cast<RxThread *>(workerThreads[id]);

	} while (thread == NULL);

	if (!thread->push(in)) {
		PLOG(logDEBUG3) << "Message " << copy << " lost when push failed";
	}
}

void TmxMessageManager::IncomingMessage(const tmx::byte_stream &bytes, const char *encoding, tmx::byte_t groupId, tmx::byte_t uniqId, uint64_t timestamp) {
	this->IncomingMessage(bytes.data(), bytes.size(), encoding, groupId, uniqId, timestamp);
}

void TmxMessageManager::IncomingMessage(std::string strBytes, const char *encoding, byte_t groupId, byte_t uniqId, uint64_t timestamp) {
	string enc = (encoding ? encoding : "");

	if (IsByteHexEncoded(enc))
		this->IncomingMessage(byte_stream_decode(strBytes), encoding, groupId, uniqId, timestamp);
	else
		this->IncomingMessage((byte_t *)strBytes.data(), strBytes.size(), encoding, groupId, uniqId, timestamp);
}

void TmxMessageManager::OutgoingMessage(const tmx::routeable_message &msg, bool immediate) {
	if (immediate) {
		this->BroadcastMessage(msg);
		return;
	}

	// At worst case, round robin the assignments
	static int lastId = 0;

	int id = workerThreads.this_thread();
	if (id < 0) {
		id = lastId++;

		if (lastId >= workerThreads.size())
			lastId = 0;
	}

	stringstream ss;
	ss << msg;

	MessageStruct out;
	out.groupId = 0;
	out.uniqId = 0;
	out.encoding = strdup(messages::api::ENCODING_JSON_STRING);
	out.mgr = this;
	out.msgLen = ss.str().length();
	out.msgBytes = (tmx::byte_t *) calloc(out.msgLen, sizeof(tmx::byte_t));
	if (out.msgBytes)
		memcpy(out.msgBytes, ss.str().c_str(), out.msgLen);

	PLOG(logDEBUG4) << "Outgoing encoding ptr is '" << (uint64_t)out.encoding << "'";
	PLOG(logDEBUG4) << "Outgoing msgBytes ptr is '" << (uint64_t)out.msgBytes << "'";

	RxThread *thread = dynamic_cast<RxThread *>(workerThreads[id]);
	if (thread)
	{
		if (thread->push_out(out)) {
			OutputThread::instance().notify();
		} else {
			PLOG(logWARNING) << "Message " << msg << " lost when push failed";
		}
	}

	this_thread::yield();
}

void TmxMessageManager::OnMessageReceived(const tmx::routeable_message &msg) {
	return PluginClient::OnMessageReceived(const_cast<IvpMessage *>(msg.get_message()));
}

void TmxMessageManager::OnMessageReceived(IvpMessage *msg) {
	if (msg)
		this->IncomingMessage(msg, 0, 0, msg->timestamp);
}

void TmxMessageManager::OnConfigChanged(const char *key, const char *value) {
	if (strcmp(NUMBER_WORKER_THREADS_CFG, key) == 0) {
		size_t n = strtoul(value, NULL, 0);
		if (n > _numThreads && n < 512) {
			_numThreads = n;

			// Start new threads
			Start();
		}
	} else if (strcmp(ASSIGNMENT_STRATEGY_CFG, key) == 0) {
		threadAssign.set_strategy(value);
	} else if (strcmp(OVERFLOW_CAPACITY_CFG, key) == 0) {
		uint16_t n = strtoul(value, NULL, 0);
		if (n != overflow)
			overflow = n;
	} else {
		// Not my key
		PluginClient::OnConfigChanged(key, value);
	}
}

void TmxMessageManager::OnStateChange(IvpPluginState state) {
	PluginClient::OnStateChange(state);

	if (state == IvpPluginState_registered) {
		// Initialize the manager config data
		_numThreads = DEFAULT_NUMBER_WORKER_THREADS;
		GetConfigValue(NUMBER_WORKER_THREADS_CFG, _numThreads);

		string s = DEFAULT_ASSIGNENT_STRATEGY;
		GetConfigValue(ASSIGNMENT_STRATEGY_CFG, s);
		threadAssign.set_strategy(s);

		GetConfigValue(OVERFLOW_CAPACITY_CFG, overflow);

		// Start the new threads
		Start();
	}
}

bool TmxMessageManager::IsRunning() {
	lock_guard<mutex> lock(_threadLock);

	if (!workerThreads.IsRunning()) return false;

	return run;
}

void TmxMessageManager::Start() {
	auto &ot = OutputThread::instance();

	ot.manager = this;

	if (!ot.IsRunning()) {
		PLOG(logDEBUG) << "Starting output thread";
		ot.Start();
	}

	size_t n = _numThreads;

	PLOG(logDEBUG) << "Starting " << n << " message manager worker threads";

	lock_guard<mutex> lock(_threadLock);

	threadAssign.Group() = workerThreads;

	// Only support an increase in the number threads
	while (n > workerThreads.size())
		workerThreads.push_back(new RxThread());

	workerThreads.Start();
}

void TmxMessageManager::Stop() {
	lock_guard<mutex> lock(_threadLock);

	workerThreads.Stop();

	auto &outThread = OutputThread::instance();
	if (outThread.Joinable())
		outThread.Join();
}

}} // namespace tmx::utils
