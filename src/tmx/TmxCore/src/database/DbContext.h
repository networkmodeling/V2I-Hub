/*
 * DbContext.h
 *
 *  Created on: Jul 23, 2014
 *      Author: ivp
 */

#ifndef DBCONTEXT_H_
#define DBCONTEXT_H_

#include <string.h>

#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/warning.h>
#include <cppconn/metadata.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/resultset.h>
#include <cppconn/resultset_metadata.h>
#include <cppconn/statement.h>
#include <database/DbConnectionPool.h>

typedef sql::SQLException DbException;

class DbContext {
public:
	virtual ~DbContext();

	static tmx::utils::DbConnectionInformation ConnectionInformation;

	tmx::utils::DbConnection getConnection();
protected:
	DbContext();

	static std::string formatStringValue(std::string str);
private:
	tmx::utils::DbConnectionPool ConnectionPool;
};


#endif /* DBCONTEXT_H_ */
