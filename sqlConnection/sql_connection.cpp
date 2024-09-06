
#include "sql_connection.h"

// handles and variables
sql::Connection *db_connection;
SQLHANDLE sqlConnHandle;
SQLHANDLE sqlStmtHandle;
SQLHANDLE sqlEnvHandle;
//const char* event_log_source_name("cnc_current_tracker_service");

void DatabaseManager::connectToSQLServer() {
    //HANDLE event_log = RegisterEventSource(NULL, event_log_source_name);
    SQLCHAR dsn_buff[256];
    SQLCHAR desc[256];
    SQLSMALLINT dsn_ret;
    SQLSMALLINT desc_ret;
    SQLUSMALLINT direction;
    SQLRETURN ret;
    SQLCHAR retconstring[SQL_RETURN_CODE_LEN];
    // init handles
    sqlConnHandle = NULL;
    sqlStmtHandle = NULL;
    // allocations
    if (SQL_SUCCESS != SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &sqlEnvHandle))
        MessageBox(NULL, " connected to database\n", NULL, NULL);
    if (SQL_SUCCESS != SQLSetEnvAttr(sqlEnvHandle, SQL_ATTR_ODBC_VERSION, (SQLPOINTER)SQL_OV_ODBC3, 0))
        MessageBox(NULL, " connected to database\n", NULL, NULL);
    if (SQL_SUCCESS != SQLAllocHandle(SQL_HANDLE_DBC, sqlEnvHandle, &sqlConnHandle))
        MessageBox(NULL, " connected to database\n", NULL, NULL);

    direction = SQL_FETCH_FIRST;
    std::cout << "SQL DATA SOURCES:" << std::endl;
    while (SQL_SUCCEEDED(ret = SQLDataSources(sqlEnvHandle, direction,
        dsn_buff, sizeof(dsn_buff), &dsn_ret,
        desc, sizeof(desc), &desc_ret))) {
        direction = SQL_FETCH_NEXT;
        std::cout << dsn_buff << " | " << desc << std::endl;
        if (ret == SQL_SUCCESS_WITH_INFO) printf("\tdata truncation\n");
    }

    HKEY hKey = OpenKey(HKEY_LOCAL_MACHINE, "SOFTWARE\\CNC_Current_Tracker");
    
    std::stringstream conn_string;
    conn_string << "SERVER=" << GetStrVal(hKey, "HOST", REG_SZ) << ";";
    conn_string << "DSN=" << GetStrVal(hKey, "DSN", REG_SZ) << ";";
    conn_string << "DATABASE=" << GetStrVal(hKey, "SCHEMA", REG_SZ) << ";";
    conn_string << "UID=" << GetStrVal(hKey, "Username", REG_SZ) << ";";
    conn_string << "PWD=" << GetStrVal(hKey, "Password", REG_SZ)<< ";";

    SvcReportEvent("SQL_Connection", "Attempting to connect to SQL SERVER");
    //with string : "+conn_string.str()

    const char* success_message = "Successfully connected to SQL Server";
    const char* success_message_with_info = "Successfully connected to SQL Server. There was additional details";
    const char* invalid_handle_message = "Could not connect to SQL Server";
    SQLSMALLINT msg_len = 0;
    SQLCHAR sql_state[6];
    SQLCHAR message[256];
    SQLINTEGER native_error = 0;

    switch (SQLDriverConnect(sqlConnHandle,
        NULL,
        (SQLCHAR*)conn_string.str().c_str(),
        SQL_NTS,
        retconstring,
        sizeof(retconstring),
        NULL,
        SQL_DRIVER_NOPROMPT
    )) {
    case SQL_SUCCESS:
        std::cout << success_message << "\n";
        SvcReportEvent("SQL_Connection", success_message, 0);
        //ReportEvent(event_log, EVENTLOG_SUCCESS, 0, 0, NULL, 1, 0, &success_message, NULL);
        break;
    case SQL_SUCCESS_WITH_INFO:
        std::cout << success_message_with_info << "\n";
        SvcReportEvent("SQL_Connection", success_message_with_info, 0);
        //ReportEvent(event_log, EVENTLOG_SUCCESS, 0, 0, NULL, 1, 0, &success_message_with_info, NULL);
        break;
    case SQL_INVALID_HANDLE:
        std::cout << invalid_handle_message << "\n";
        //ReportEvent(event_log, EVENTLOG_ERROR_TYPE, 0, 0, NULL, 1, 0, &invalid_handle_message, NULL);
        SvcReportEvent("SQL_Connection", invalid_handle_message);
        CleanupHandlers();
    case SQL_ERROR:
        SQLGetDiagRec(SQL_HANDLE_DBC, sqlConnHandle, 1, sql_state, &native_error, message, sizeof(message), &msg_len);
        std::cout << "Could not connect to SQL Server" << " Error code: " << sql_state << " Message: " << message << "\n";
        SvcReportEvent("SQL_Connection", "Could not connect to SQL Server Error Message: " + std::string((const char *)message));
        //ReportEvent(event_log, EVENTLOG_ERROR_TYPE, 0, 0, NULL, 1, 0, message, NULL);
        CleanupHandlers();
    default:
        break;
    }
    RegCloseKey(hKey);
    // exit if error with connecting
    if (SQL_SUCCESS != SQLAllocHandle(SQL_HANDLE_STMT, sqlConnHandle, &sqlStmtHandle))
        CleanupHandlers();
}

void DatabaseManager::CleanupHandlers() {
    //close connection and free resources
    SQLFreeHandle(SQL_HANDLE_STMT, sqlStmtHandle);
    SQLDisconnect(sqlConnHandle);
    SQLFreeHandle(SQL_HANDLE_DBC, sqlConnHandle);
    SQLFreeHandle(SQL_HANDLE_ENV, sqlEnvHandle);
    //pause the console window - exit when key is pressed
    /*std::cout << "\nPress any key to exit...";
    getchar();*/
}

void DatabaseManager::setupSQLServerTable() {
    std::cout << "\n" << "Executing T-SQL query..." << "\n";

    const char* query = "IF NOT EXISTS (SELECT * FROM INFORMATION_SCHEMA.TABLES WHERE TABLE_NAME = N'cnc_current')\n\
        BEGIN\n\
        CREATE TABLE dbo.cnc_current\n\
        (\n\
            Id INT NOT NULL IDENTITY(1, 1) PRIMARY KEY,\n\
            Date DATE DEFAULT GETDATE(),\n\
            Time TIME DEFAULT CURRENT_TIMESTAMP,\n\
            SourceAddress VARCHAR(50),\n\
            NodeId INT,\n\
            FirmwareVersion INT,\n\
            BatteryVolt FLOAT,\n\
            PacketCount INT,\n\
            SensorType INT,\n\
            Channel1Name VARCHAR(50),\n\
            Channel1Current FLOAT,\n\
            Channel2Name VARCHAR(50),\n\
            Channel2Current FLOAT\n\
        )\n\
        END";
    //if there is a problem executing the query then exit application
    //else display query result
    if (SQL_SUCCESS != SQLExecDirect(sqlStmtHandle, (SQLCHAR*)query, SQL_NTS)) {
        std::cout << "Error querying SQL Server" << "\n";
        SvcReportEvent("SQL_Connection", "Error creating table in SQL Server");
        CleanupHandlers();
    }
    else {
        //declare output variable and pointer
        SQLCHAR sqlVersion[SQL_RESULT_LEN];
        SQLLEN ptrSqlVersion;
        while (SQLFetch(sqlStmtHandle) == SQL_SUCCESS) {
            SQLGetData(sqlStmtHandle, 1, SQL_CHAR, sqlVersion, SQL_RESULT_LEN, &ptrSqlVersion);
            //display query result
            std::cout << "\nQuery Result:\n\n" << sqlVersion << std::endl;
        }
    }
}

void DatabaseManager::AddReadingToSqlServer(struct CNCVoltageMeter &measures)
{
    std::cout << "\n" << "Executing T-SQL query..." << "\n";

    std::string query = "INSERT INTO cnc_current(\n\
    SourceAddress,\n\
    NodeId,\n\
    FirmwareVersion,\n\
    BatteryVolt,\n\
    PacketCount,\n\
    SensorType,\n\
    Channel1Name,\n\
    Channel1Current,\n\
    Channel2Name,\n\
    Channel2Current\n\
) VALUES(\n\
    '" + measures.sourceAddress + "',\n\
    " + std::to_string(measures.nodeId) + ",\n\
    " + std::to_string(measures.firmwareVersion) + ",\n\
    " + std::to_string(measures.batteryVoltage) + ",\n\
    " + std::to_string(measures.packetCounter) + ",\n\
    " + std::to_string(measures.sensorType) + ",\n\
    '" + measures.channel1Name + "',\n\
    " + std::to_string(measures.channel1Reading) + ",\n\
    '" + measures.channel2Name + "',\n\
    " + std::to_string(measures.channel2Reading) + "\n\
);";
    //if there is a problem executing the query then exit application
    //else display query result
    if (SQL_SUCCESS != SQLExecDirect(sqlStmtHandle, (SQLCHAR*)query.c_str(), SQL_NTS)) {
        std::cout << "Error querying SQL Server" << "\n";
        SvcReportEvent("SQL_Connection", "Error creating entry in SQL Server");
        CleanupHandlers();
    }
    else {
        //declare output variable and pointer
        SQLCHAR sqlVersion[SQL_RESULT_LEN];
        SQLLEN ptrSqlVersion;
        while (SQLFetch(sqlStmtHandle) == SQL_SUCCESS) {
            SQLGetData(sqlStmtHandle, 1, SQL_CHAR, sqlVersion, SQL_RESULT_LEN, &ptrSqlVersion);
            //display query result
            std::cout << "\nQuery Result:\n\n" << sqlVersion << std::endl;
        }
    }
}

// connect to mariadb Process
int DatabaseManager::connect() {
    
    sql::Driver* driver = sql::mariadb::get_driver_instance();

    // Configure Connection
    //sql::SQLString url = "jdbc:mariadb://"+ip+":"+port+"/"+db_name;
    char database_host[1024];
    char database_DNS[1024];
    char database_schema[1024];
    char database_user[1024];
    char database_pass[1024];
    int database_port = GetPrivateProfileInt("Database", "Port", 3306, ".\\settings.ini");
    GetPrivateProfileString("Database", "Host", NULL, database_host, sizeof(database_host) / sizeof(database_host[0]), ".\\settings.ini");
    GetPrivateProfileString("Database", "DNS", NULL, database_DNS, sizeof(database_DNS) / sizeof(database_DNS[0]), ".\\settings.ini");
    GetPrivateProfileString("Database", "Schema", NULL, database_schema, sizeof(database_schema) / sizeof(database_schema[0]), ".\\settings.ini");
    GetPrivateProfileString("Database", "Username", NULL, database_user, sizeof(database_user) / sizeof(database_user[0]), ".\\settings.ini");
    GetPrivateProfileString("Database", "Password", NULL, database_pass, sizeof(database_pass) / sizeof(database_pass[0]), ".\\settings.ini");
    sql::SQLString url = "jdbc:sqlserver://" + (std::string)database_host + ":" + std::to_string(database_port) + "/" + (std::string)database_schema;
    sql::Properties properties({
        {"hostName", "jdbc:mariadb://" + (std::string)database_host + ":" + std::to_string(database_port)},
        {"schema", database_schema},
        {"user", database_user},
        {"password", database_pass},
    });
    // Establish Connection

    try {
        //db_connection = (sql::Connection*)driver->connect(properties);
        db_connection = (sql::Connection *)sql::DriverManager::getConnection(url, properties);
    }
    catch (sql::SQLException e) {
        std::cerr << "Error Connecting to the database: "
            << e.what() << std::endl;

        return 1;
    }
    // Exit (Success)
    return 0;
}

sql::Connection *DatabaseManager::getConnection() {
    sql::Connection *conn = db_connection;
    return conn;
}

void DatabaseManager::setupTable(sql::Connection& conn) {
    sql::Connection* db = &conn;
    sql::SQLString table = db->getSchema();
    //db->setAutoCommit(false);
    try {
        std::unique_ptr<sql::Statement> stmnt(db->createStatement());
        std::cout << table << std::endl; 

        stmnt->executeQuery("CREATE TABLE IF NOT EXISTS machine_currents (\n\
Id INT UNSIGNED NOT NULL PRIMARY KEY AUTO_INCREMENT, \n\
Date DATE DEFAULT CURRENT_DATE(), \n\
Time TIME DEFAULT CURRENT_TIME(), \n\
SourceAddress VARCHAR(50) CHARACTER SET utf8, \n\
NodeId INT UNSIGNED, \n\
FirmwareVersion INT UNSIGNED, \n\
BatteryVolt DOUBLE, \n\
PacketCount INT UNSIGNED, \n\
SensorType INT UNSIGNED, \n\
Channel1Name VARCHAR(50) CHARACTER SET utf8, \n\
Channel1Current DOUBLE, \n\
Channel2Name VARCHAR(50) CHARACTER SET utf8, \n\
Channel2Current DOUBLE \n\
)");
    }
    catch (sql::SQLException &e) {
        std::cerr << "Error creating table: "
            << e.what() << std::endl;
        SvcReportEvent("SQL_Connection");
        db->rollback();
    }
}

void DatabaseManager::addReading(struct CNCVoltageMeter &measures) {
    sql::Connection* db = db_connection;

    std::unique_ptr<sql::PreparedStatement> stmnt(db->prepareStatement(
        "INSERT INTO machine_currents (Date, Time, SourceAddress, NodeId, FirmwareVersion, BatteryVolt, PacketCount, SensorType, Channel1Name, Channel1Current, Channel2Name, Channel2Current) VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)"
    ));

    try {
        stmnt->setString(1, measures.date);
        stmnt->setString(2, measures.time);
        stmnt->setString(3, measures.sourceAddress);
        stmnt->setInt(4, measures.nodeId);
        stmnt->setInt(5, measures.firmwareVersion);
        stmnt->setDouble(6, measures.batteryVoltage);
        stmnt->setInt(7, measures.packetCounter);
        stmnt->setInt(8, measures.sensorType);
        stmnt->setString(9, measures.channel1Name);
        stmnt->setDouble(10, measures.channel1Reading);
        stmnt->setString(11, measures.channel2Name);
        stmnt->setDouble(12, measures.channel2Reading);

        stmnt->executeQuery();
    }
    catch (sql::SQLException& e) {
        std::cerr << "Error inserting values into table: "
            << e.what() << std::endl;
        SvcReportEvent("SQL_Connection");
    }
}



// Delete a task record (indicated by id)
void DatabaseManager::deleteTask(std::unique_ptr<sql::Connection>& conn, int id) {
    try {
        // Create a new PreparedStatement
        std::unique_ptr<sql::PreparedStatement> stmnt(conn->prepareStatement("delete from tasks where id = ?"));
        // Bind values to SQL statement
        stmnt->setInt(1, id);
        // Execute query
        stmnt->executeQuery();
    }
    catch (sql::SQLException& e) {
        std::cerr << "Error deleting task: " << e.what() << std::endl;
        SvcReportEvent("SQL_Connection");
    }
}

// Update the completed value of a task record (indicated by id)
void DatabaseManager::updateTaskStatus(std::unique_ptr<sql::Connection>& conn, int id, bool completed) {
    try {
        // Create a new PreparedStatement
        std::unique_ptr<sql::PreparedStatement> stmnt(conn->prepareStatement("update tasks set completed = ? where id = ?"));
        // Bind values to SQL statement
        stmnt->setBoolean(1, completed);
        stmnt->setInt(2, id);
        // Execute query
        stmnt->executeQuery();
    }
    catch (sql::SQLException& e) {
        std::cerr << "Error updating task status: " << e.what() << std::endl;
        SvcReportEvent("SQL_Connection");
    }
}

// Create a new task record
void DatabaseManager::addTask(std::unique_ptr<sql::Connection>& conn, std::string description) {
    try {
        // Create a new PreparedStatement
        std::unique_ptr<sql::PreparedStatement> stmnt(conn->prepareStatement("insert into tasks (description) values (?)"));
        // Bind values to SQL statement
        stmnt->setString(1, description);
        // Execute query
        stmnt->executeQuery();
    }
    catch (sql::SQLException& e) {
        std::cerr << "Error inserting new task: " << e.what() << std::endl;
        SvcReportEvent("SQL_Connection");
    }
}

// Print all records in tasks table 
void DatabaseManager::showTasks(std::unique_ptr<sql::Connection>& conn) {
    try {
        // Create a new Statement
        std::unique_ptr<sql::Statement> stmnt(conn->createStatement());
        // Execute query
        sql::ResultSet* res = stmnt->executeQuery("select * from tasks");
        // Loop through and print results
        while (res->next()) {
            std::cout << "id = " << res->getInt(1);
            std::cout << ", description = " << res->getString(2);
            std::cout << ", completed = " << res->getBoolean(3) << "\n";
        }
    }
    catch (sql::SQLException& e) {
        std::cerr << "Error selecting tasks: " << e.what() << std::endl;
        SvcReportEvent("SQL_Connection");
    }
}

