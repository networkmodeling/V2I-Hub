//============================================================================
// Name        : LocationPlugin.cpp
// Author      : Battelle Memorial Institute - Matt Cline (cline@battelle.org)
// Version     :
// Copyright   : Battelle 2016
// Description : Plugin to send location messages after gathering the information
//				 from gpsd.
//============================================================================

#include "LocationPlugin.h"

//#include "GPSDControl.h"

#include <Clock.h>
#include <PluginClient.h>
#include <LocationMessage.h>

using namespace std;
using namespace tmx;
using namespace tmx::utils;
using namespace tmx::messages;
using namespace tmx::messages::location;

namespace LocationPlugin{

/**
 * Default Constructor. Good Place to initialize atomic variables.
 *
 * @param name string used to identify the plugin.
 */
LocationPlugin::LocationPlugin(std::string name) : TmxMessageManager(name)
{
	_sendNmea = false;
}

/**
 * Default Deconstructor.
 */
LocationPlugin::~LocationPlugin()
{

}

/**
 * Called to update the configuration parameters stored in the database.
 */
void LocationPlugin::UpdateConfigSettings()
{
	{
		// Updating the source must be protected
		lock_guard<mutex> lock(_dataLock);
		GetConfigValue("GPSSource", _gpsdHost);
	}

	// These are all atomic updates
	GetConfigValue("LatchSpeed", _latchSpeed);
	GetConfigValue("SendRawNMEA", _sendNmea);

	ConfigureDevice();
}

/**
 * Function called when a configuration parameter is updated
 *
 * @param key the name of the parameter updated
 * @param value the new value of the parameter
 */
void LocationPlugin::OnConfigChanged(const char* key, const char* value)
{
	TmxMessageManager::OnConfigChanged(key, value);

	if (IsPluginState(IvpPluginState_registered))
		UpdateConfigSettings();
}

/**
 * Function called when the state of the plugin changes
 *
 * @param state the new state of the plugin
 */
void LocationPlugin::OnStateChange(IvpPluginState state)
{
	TmxMessageManager::OnStateChange(state);

	if (state == IvpPluginState_registered)
	{
		UpdateConfigSettings();
	}
}

/**
 * Main Function logic to execute on a separate thread
 *
 * @return exit code
 */
int LocationPlugin::Main()
{
	PLOG(logINFO) << "Starting Plugin.";

	while (!IsPluginState(IvpPluginState_registered))
		sleep(2);

	while (!IsPluginState(IvpPluginState_error))
		this->SampleGPS();

	return 0;
}


} /* namespace LocationPlugin */

int main(int argc, char* argv[])
{
	return run_plugin<LocationPlugin::LocationPlugin>("Location", argc, argv);
}
