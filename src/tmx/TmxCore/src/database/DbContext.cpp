/*
 * DbContext.cpp
 *
 *  Created on: Jul 23, 2014
 *      Author: ivp
 */

#include "DbContext.h"
#include <sstream>

tmx::utils::DbConnectionInformation DbContext::ConnectionInformation;

DbContext::DbContext() { }

DbContext::~DbContext() { }

tmx::utils::DbConnection DbContext::getConnection() {
	return this->ConnectionPool.Connection(ConnectionInformation);
}

std::string DbContext::formatStringValue(std::string str)
{
	std::stringstream ss;

	for(unsigned int i = 0; i < str.length(); i++)
	{
		char c = str.at(i);
		if (c == '\'')
			ss << "\\'";
		else
			ss << c;
	}

	return ss.str();
}

