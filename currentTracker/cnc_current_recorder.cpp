// CNCVoltageTracker.cpp : Defines the entry point for the application.
//


#include "cnc_current_recorder.h"

using namespace std;
DatabaseManager database;

//std::string app_path;
//std::stringstream logx;

std::string GetLogsPath() {
    std::string logsPath;
    int _results = 0;
    char buff[1024];
    if (app_path == "") {
        do {
            _results = GetCurrentDirectory(sizeof(buff), buff);
            logsPath = buff;
        } while (_results > logsPath.length() && logsPath.data());
    }
    else {
        logsPath = app_path.substr(0, app_path.find_last_of("/\\"));
    }
    logsPath.append("\\logs\\");
    if (!std::filesystem::is_directory(logsPath.c_str())) {
        std::filesystem::create_directory(logsPath.c_str());
    }
    return logsPath;
}

void WriteToLog(std::string log = "") {
    if (log == "") {
        log = logx.str();
    }
    std::string logDir = GetLogsPath();
    logDir.append("log.txt");
    std::fstream fs(logDir.c_str(), std::ios::out | std::ios_base::app);
    if (fs) {
        time_t timer = time(NULL);
        struct tm currDateTime = *localtime(&timer);
        char dateBuf[120];
        char timeBuf[120];
        strftime(dateBuf, sizeof(dateBuf), "%d/%m/%Y", &currDateTime);
        strftime(timeBuf, sizeof(timeBuf), "%T", &currDateTime);
        fs << "###-< " << dateBuf << " " << timeBuf << " >-###\n\n";
        fs << log << '\n';
        fs.close();
    }
    logx.str(std::string());
}

int cnc_current_recorder(HANDLE *g_ServiceStopEvent)
{
    //----------------------
    // Declare and initialize variables.
    HKEY hKey = OpenKey(HKEY_LOCAL_MACHINE, "SOFTWARE\\CNC_Current_Tracker");
    std::string installPath = GetStrVal(hKey, "InstallPath", REG_SZ);
    app_path = installPath+"\\";
    logx << "Retrieved App Path from Registry: " << app_path.data() << std::endl;
    SvcReportEvent("cnc_current_recorder", "Retrieved App Path from Registry: "+ app_path);
    CNCVoltageMeter reading;
    
    WSADATA wsaData;
    int iResult;

    SOCKET ConnectSocket = INVALID_SOCKET;
    struct sockaddr_in clientService;

    char recvbuf[DEFAULT_BUFLEN];
    int recvbuflen = DEFAULT_BUFLEN;

    ofstream outputLog;
    //--------------------
        // Connect to target database
    logx << "Connecting to SQL Server" << std::endl;
    database.connectToSQLServer();
    //try {
    //	//database.connect();
    //}
    //catch (sql::SQLException& e) {
    //	std::cerr << "Error Connecting to the database: "
    //		<< e.what() << std::endl;
    //	return 1;
    //}
    //sql::Connection* dbConn = database.getConnection();
    //if (!dbConn) {
    //    std::cout << "could not connect\n";
    //    //std::cout << dbConn->getSchema() << std::endl;
    //    return 1;
    //}
    //std::cout << "Successfullt Connected To: " << dbConn->getSchema() << "\n";
    //database.setupTable(*dbConn);
    logx << "Setting up table in database" << std::endl;
    database.setupSQLServerTable();
    ////----------------------
    //// Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != NO_ERROR) {
        printf("WSAStartup failed: %d\n", iResult);
        SvcReportEvent("cnc_current_recorder");
        logx << "Failed to startup WSA" << std::endl;
        WriteToLog();
        return 1;
    }

    ////----------------------
    //// Create a SOCKET for connecting to server
    ConnectSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (ConnectSocket == INVALID_SOCKET) {
        printf("Error at socket(): %ld\n", WSAGetLastError());
        SvcReportEvent("cnc_current_recorder");
        logx << "Failed to create socket" << std::endl;
        WriteToLog();
        WSACleanup();
        return 1;
    }

    ////----------------------
    //// The sockaddr_in structure specifies the address family,
    //// IP address, and port of the server to be connected to.
    clientService.sin_family = AF_INET;
    //clientService.sin_addr.s_addr = inet_addr("192.168.2.36");
    inet_pton(AF_INET, DEFAULT_IP, (SOCKADDR*)&clientService.sin_addr.s_addr);
    clientService.sin_port = htons(2101);

    ////----------------------
    //// Connect to server.
    
    iResult = 1;
    do {
        logx << "Connecting to socket" << std::endl;
        iResult = connect(ConnectSocket, (SOCKADDR*)&clientService, sizeof(clientService));
        if (iResult == SOCKET_ERROR) {
            // closesocket(ConnectSocket);
            printf("Unable to connect to server: %ld\n", WSAGetLastError());
            SvcReportEvent("cnc_current_recorder");
            //printf("Retrying connection in 5 seconds");
            logx << "Failed to connect socket" << std::endl;
            WriteToLog();
            this_thread::sleep_for(chrono::minutes(1));
            //WSACleanup();
            //return 1;
        }
        else {
            std::cout << "Connected to: " << DEFAULT_IP << " on port: " << DEFAULT_PORT << endl;
        }
    } while (iResult != 0 || !g_ServiceStopEvent);
    string logFileName = GetLogsPath() + "cnc_current_output_log.csv";
    SvcReportEvent("cnc_current_recorder", "Creating file at: "+logFileName);
    // Receive until the peer closes the connection
    outputLog.open(logFileName, fstream::in | fstream::out);
    string headers = "Date, Time, SourceAddress, Node Id, Firmware Version, Battery Voltage, Packet Count, Sensor Type, Channel 1 Name, Channel 1 Current, Channel 2 Name, Channel 2 Current\n";
    if (!outputLog.is_open()) {
        std::cout << "Cannot open log file, log file does not exist. Creating new log file..";
        outputLog.open(logFileName, fstream::in | fstream::out | fstream::trunc);
        outputLog << headers;
        outputLog.close();
    }
    // look into setting header regardless of when file is opened
    if (outputLog.is_open()) {
        std::cout << "updating headers..." << endl;
        outputLog << headers;
        outputLog.close();
    }
    do {
        iResult = recv(ConnectSocket, recvbuf, recvbuflen, MSG_WAITALL);
        //cout << "testlog\n";
        if (iResult > 0) {
            outputLog.open(logFileName, fstream::in | fstream::out | fstream::app);
            printf("Bytes received: %d\n", iResult);
            time_t timestamp = time(NULL);
            struct tm tstruct = *localtime(&timestamp);
            char dateBuf[120];
            char timeBuf[120];
            strftime(dateBuf, sizeof(dateBuf), "%F", &tstruct);
            strftime(timeBuf, sizeof(timeBuf), "%T", &tstruct);
            //if (time[strlen(time) - 1] == '\n') time[strlen(time) - 1] = '\0';
            reading.date = dateBuf;
            reading.time = timeBuf;

            for (int i = 0; i < sizeof(recvbuf); i++)
                std::cout << hex << setw(2) << setfill('0') << static_cast<int>((UINT8)recvbuf[i]) << ' ';
            std::cout << endl;
            std::cout << "-------" << endl;

            // date of reading
            outputLog << reading.date << ",";
            std::cout << "Date: " << reading.date << endl;

            // time of reading
            outputLog << reading.time << ",";
            std::cout << "Date: " << reading.time << endl;

            // sensor data is cominging from
            reading.sourceAddress = "";
            for (int i = 4; i < 12; i++) {
                ostringstream string;
                string << hex << setw(2) << setfill('0') << static_cast<int>((UINT8)recvbuf[i]);
                reading.sourceAddress += string.str();
            }
            outputLog << reading.sourceAddress << ",";
            std::cout << "sourceAddress: " << reading.sourceAddress << endl;

            reading.channel1Name = "Chnl2";
            reading.channel2Name = "Chnl2";
            if (reading.sourceAddress == "0013a200422647a4") {
                reading.channel1Name = "CNC1";
                reading.channel2Name = "CNC3";
            }
            else if (reading.sourceAddress == "0013a20042264583") {
                reading.channel1Name = "CNC4";
                reading.channel2Name = "CNC2";
            }

            // header node
            //cout << "header: " << dec << setw(2) << setfill('0') << static_cast<int>((UINT8)recvbuf[15]) << endl;

            // node id
            ostringstream nodeId;
            nodeId << dec << setw(2) << setfill('0') << static_cast<int>((UINT8)recvbuf[16]);
            reading.nodeId = std::stoi(nodeId.str());
            outputLog << reading.nodeId << ",";
            std::cout << "node id: " << reading.nodeId << endl;

            // firmware version
            ostringstream firmware;
            firmware << dec << setw(2) << setfill('0') << static_cast<int>((UINT8)recvbuf[17]);
            reading.firmwareVersion = std::stoi(firmware.str());
            outputLog << reading.firmwareVersion << ",";
            std::cout << "firmware: " << reading.firmwareVersion << endl;

            // battery voltage
            reading.batteryVoltage = (((UINT8)recvbuf[18] * 256) + (UINT8)recvbuf[19]) * 0.00322;
            outputLog << reading.batteryVoltage << ",";
            std::cout << "battery voltage: " << reading.batteryVoltage << endl;

            // packet count
            ostringstream packetCount;
            packetCount << dec << setw(2) << setfill('0') << static_cast<int>((UINT8)recvbuf[17]);
            reading.packetCounter = std::stoi(packetCount.str());
            outputLog << reading.packetCounter << ",";
            std::cout << "Packet Counter: " << reading.packetCounter << endl;

            // Sensor Type
            reading.sensorType = (int)recvbuf[21] + (int)recvbuf[22];
            outputLog << reading.sensorType << ",";
            std::cout << "Sensor Type: " << reading.sensorType << endl;

            // Voltage 1
            reading.channel1Reading = (((UINT8)recvbuf[24] << 16) + ((UINT8)recvbuf[25] << 8) + (UINT8)recvbuf[26]) / 1000.00;
            outputLog << reading.channel1Name << "," << reading.channel1Reading << ",";
            std::cout << reading.channel1Name << ": " << reading.channel1Reading << endl;

            // Voltage 2
            reading.channel2Reading = (((UINT8)recvbuf[28] << 16) + ((UINT8)recvbuf[29] << 8) + (UINT8)recvbuf[30]) / 1000.00;
            outputLog << reading.channel2Name << "," << reading.channel2Reading << "\n";
            std::cout << reading.channel2Name << ": " << reading.channel2Reading << endl;
            std::cout << "-------" << endl;
            outputLog.close();
            //database.addReading(reading);
            if ((tstruct.tm_wday != 0 && tstruct.tm_wday != 6) && (tstruct.tm_hour >= 7 && tstruct.tm_hour <= (4 + 12)))
                database.AddReadingToSqlServer(reading);

        }
        else if (iResult == 0)
            printf("Connection closed\n");
        else
            printf("recv failed: %d\n", WSAGetLastError());
            SvcReportEvent("cnc_current_recorder");

    } while (iResult > 0 || !g_ServiceStopEvent);

    // cleanup
    //dbConn->close();
    closesocket(ConnectSocket);
    WSACleanup();
    return 0;
}
