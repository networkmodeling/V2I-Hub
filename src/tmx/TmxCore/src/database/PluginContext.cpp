/*
 * PluginContext.cpp
 *
 *  Created on: Jul 23, 2014
 *      Author: ivp
 */

#include "PluginContext.h"
#include <assert.h>
#include <sstream>

using namespace std;

PluginContext::PluginContext()
{

}

vector<PluginEntry> PluginContext::getAllPlugins()
{
	vector<PluginEntry> results;

	auto conn = this->getConnection();
	unique_ptr< sql::Statement > stmt(conn.Get()->createStatement());

	unique_ptr< sql::ResultSet > rset(stmt->executeQuery("SELECT * FROM plugin"));
	while(rset->next())
	{
		PluginEntry entry;

		entry.id = rset->getUInt("id");
		entry.name = rset->getString("name");
		entry.description = rset->getString("description");
		entry.version = rset->getString("version");

		results.push_back(entry);
	}

	return results;
}

PluginEntry PluginContext::getPlugin(std::string pluginName)
{
	PluginEntry results;

	auto conn = this->getConnection();
	unique_ptr< sql::Statement > stmt(conn.Get()->createStatement());

	string query = "SELECT * FROM `plugin` WHERE `plugin`.`name` = '" + DbContext::formatStringValue(pluginName) + "';";
	unique_ptr< sql::ResultSet > rset(stmt->executeQuery(query));
	if (rset->next())
	{
		results.id = rset->getUInt("id");
		results.name = rset->getString("name");
		results.description = rset->getString("description");
		results.version = rset->getString("version");
	}

	return results;
}

void PluginContext::insertOrUpdatePlugin(PluginEntry &entry)
{
	auto conn = this->getConnection();
	unique_ptr< sql::Statement > stmt(conn.Get()->createStatement());

	string query = "INSERT INTO plugin (name, description, version) VALUES ('" + DbContext::formatStringValue(entry.name) + "', '" + DbContext::formatStringValue(entry.description) + "', '" + DbContext::formatStringValue(entry.version) + "') ON DUPLICATE KEY UPDATE name = VALUES(name), description = VALUES(description), version = VALUES(version)";
	stmt->execute(query);

	query = "SELECT `id` FROM `plugin` WHERE `plugin`.`name` = '" + entry.name + "';";
	unique_ptr< sql::ResultSet > rset(stmt->executeQuery(query));
	if (rset->next())
	{
		entry.id = rset->getUInt("id");
	}
}

void PluginContext::removeAllNotInstalled()
{
	auto conn = this->getConnection();
	unique_ptr< sql::Statement > stmt(conn.Get()->createStatement());

	stringstream query;
	query << "DELETE FROM `plugin` WHERE `plugin`.`id` NOT IN (SELECT `installedPlugins`.`pluginId` FROM `installedPlugins`);";
	stmt->execute(query.str());
}

/*void PluginContext::clearPluginStatuses()
{
	auto_ptr<sql::Statement> stmt(this->getStatement());

	string query = "DELETE FROM `pluginStatus`";
	stmt->execute(query);
}*/

void PluginContext::setStatusForAllPlugins(std::string status)
{
	auto conn = this->getConnection();
	unique_ptr< sql::Statement > stmt(conn.Get()->createStatement());

	stringstream query;
	query << "UPDATE `pluginStatus` SET `value` = '" << DbContext::formatStringValue(status) << "' WHERE `key` = '';";
	stmt->execute(query.str());

}

void PluginContext::setPluginStatus(unsigned int pluginId, std::string status)
{
	auto conn = this->getConnection();
	unique_ptr< sql::Statement > stmt(conn.Get()->createStatement());

	stringstream query;
	query << "INSERT INTO `pluginStatus` (`pluginId`,`key`,`value`) VALUES ('" << pluginId << "', '', '" << DbContext::formatStringValue(status) << "') ON DUPLICATE KEY UPDATE value = VALUES(value);";
	stmt->execute(query.str());
}

void PluginContext::setPluginStatusItems(unsigned int pluginId, std::vector<PluginStatusItem> statusItems)
{
	auto conn = this->getConnection();
	unique_ptr< sql::Statement > stmt(conn.Get()->createStatement());

	if (statusItems.size() > 0)
	{
		stringstream query;
		query << "INSERT INTO `pluginStatus` (`pluginId`,`key`,`value`) VALUES ";

		bool first = true;
		for(vector<PluginStatusItem>::iterator itr = statusItems.begin(); itr != statusItems.end(); itr++)
		{
			if (!first)
				query << ", ";
			query << "('" << pluginId << "', '" << DbContext::formatStringValue(itr->key) << "', '" << DbContext::formatStringValue(itr->value) << "')";
			first = false;
		}

		query << " ON DUPLICATE KEY UPDATE value = VALUES(value);";

		stmt->execute(query.str());
	}
}

void PluginContext::removePluginStatusItems(unsigned int pluginId, std::vector<std::string> itemKeys)
{
	auto conn = this->getConnection();
	unique_ptr< sql::Statement > stmt(conn.Get()->createStatement());

	for(vector<string>::iterator itr = itemKeys.begin(); itr != itemKeys.end(); itr++)
	{
		stringstream query;
		query << "DELETE FROM `pluginStatus` WHERE `pluginId` = '" << pluginId << "' AND `key` = '" << DbContext::formatStringValue(*itr) << "';";
		stmt->execute(query.str());
	}
}

void PluginContext::removeAllPluginStatusItems(unsigned int pluginId)
{
	auto conn = this->getConnection();
	unique_ptr< sql::Statement > stmt(conn.Get()->createStatement());

	stringstream query;
	query << "DELETE FROM `pluginStatus` WHERE `pluginId` = '" << pluginId << "' AND `key` > '';";
	stmt->execute(query.str());
}

void PluginContext::removeAllPluginStatusItems()
{
	auto conn = this->getConnection();
	unique_ptr< sql::Statement > stmt(conn.Get()->createStatement());

	stringstream query;
	query << "DELETE FROM `pluginStatus` WHERE `key` > '';";
	stmt->execute(query.str());
}


set<InstalledPluginEntry> PluginContext::getInstalledPlugins(bool enabledOnly)
{
	set<InstalledPluginEntry> results;

	auto conn = this->getConnection();
	unique_ptr< sql::Statement > stmt(conn.Get()->createStatement());

	/*
	 * select
    		cars.ID,
    		models.model
		from
    		cars
        		join models
            		on cars.model=models.ID
	 */

	string query = "SELECT *, `installedPlugin`.`id` AS installedPluginId FROM `installedPlugin` JOIN `plugin` ON `plugin`.`id` = `installedPlugin`.`pluginId`";
	if (enabledOnly)
	{
		query += " WHERE `installedPlugin`.`enabled` = '1'";
	}

	query += ";";

	unique_ptr< sql::ResultSet > rset(stmt->executeQuery(query));
	while(rset->next())
	{
		InstalledPluginEntry entry;

		entry.id = rset->getUInt("installedPluginId");
		entry.path = rset->getString("path");
		entry.exeName = rset->getString("exeName");
		//entry.manifestName = rset->getString("manifestName");
		entry.enabled = rset->getBoolean("enabled");
		entry.maxMessageInterval = rset->getUInt("maxMessageInterval");

		entry.plugin.id = rset->getUInt("id");
		entry.plugin.name = rset->getString("name");
		entry.plugin.description = rset->getString("description");
		entry.plugin.version = rset->getString("version");

		results.insert(entry);
	}

	return results;
}

void PluginContext::disableInstalledPlugin(unsigned int id)
{
	auto conn = this->getConnection();
	unique_ptr< sql::Statement > stmt(conn.Get()->createStatement());

	stringstream query;
	query << "UPDATE `installedPlugin` SET `enabled` = '0' WHERE `id` = '" << id << "';";
	stmt->execute(query.str());
}
