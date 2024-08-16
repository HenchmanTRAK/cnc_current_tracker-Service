#pragma once
#ifndef SQL_CONNECTION_H
#define SQL_CONNECTION_H 1

#include "cnc_current_tracker.h"

#include <stdlib.h>
#include <iostream>
#include <memory>
#include <string>
#include <sstream>
#include <stdexcept>
#include <windows.h>
#include <sqlext.h>
#include <sqltypes.h>
#include <sql.h>
#include <mariadb/conncpp.hpp>

#define SQL_RESULT_LEN 240
#define SQL_RETURN_CODE_LEN 1024

class DatabaseManager {
public:
	// MAriaDB Functions
	int connect();
	sql::Connection *getConnection();
	void setupTable(sql::Connection& conn);
	void addReading(struct CNCVoltageMeter& measures);
	// SQL server Functions
	void connectToSQLServer();
	void CleanupHandlers();
	void setupSQLServerTable();
	void AddReadingToSqlServer(struct CNCVoltageMeter &measures);

private:
	// establishing global but private handlers and connections
	sql::Connection* db_connection;
	SQLHANDLE sqlConnHandle;
	SQLHANDLE sqlStmtHandle;
	SQLHANDLE sqlEnvHandle;
	
	// used for learning
	void showTasks(std::unique_ptr<sql::Connection>& conn);
	void addTask(std::unique_ptr<sql::Connection>& conn, std::string description);
	void updateTaskStatus(std::unique_ptr<sql::Connection>& conn, int id, bool completed);
	void deleteTask(std::unique_ptr<sql::Connection>& conn, int id);
};


#endif