/*
 * @file LocationPlugin.h
 *
 *  Created on: Jul 30, 2020
 *      @author: gmb
 */

#ifndef LOCATIONPLUGIN_H_
#define LOCATIONPLUGIN_H_

#include <atomic>
#include <mutex>
#include <string>

#include <LocationMessage.h>
#include <TmxMessageManager.h>

namespace LocationPlugin {

class LocationPlugin : public tmx::utils::TmxMessageManager
{
public:
	LocationPlugin(std::string name);
	~LocationPlugin();
	int Main();

	// GPS Handlers
	void SampleGPS();

	// The default sampler does nothing and
	// therefore must be specialized
	template <uint32_t _T>
	void DoSampleGPS() {}

	void Cleanup(tmx::byte_t groupId = 0, tmx::byte_t uniqId = 0);
	bool Accept(tmx::byte_t groupId = 0, tmx::byte_t uniqId = 0);
protected:
	void UpdateConfigSettings();

	// virtual function overrides
	void OnConfigChanged(const char* key, const char* value);
	void OnStateChange(IvpPluginState state);

	// Message handlers
	void OnMessageReceived(const tmx::routeable_message &msg);

private:
	std::mutex _dataLock;

	std::string _gpsdHost;
	std::atomic<bool> _sendNmea;
	std::atomic<double> _latchSpeed;

	void ConfigureDevice();
};

} /* End namespace LocationPlugin */


#endif /* LOCATIONPLUGIN_H_ */
