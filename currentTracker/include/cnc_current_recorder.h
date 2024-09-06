// CNCVoltageTracker.h : Include file for standard system include files,
// or project specific include files.
#ifndef CNC_CURRENT_RECORDER_H
#define CNC_CURRENT_RECORDER_H	1

#pragma once

#include <iostream>
//#include <winsock2.h>
#include <Ws2tcpip.h>
#include <WinUser.h>
#include <stdio.h>
#include <iomanip>
#include <string>
#include <sstream>
#include <time.h>
#include <fstream>
#include <thread>
#include <chrono>
#include <filesystem>

#include "event_reporter.h"
#include "sql_connection.h"

#pragma comment(lib, "Ws2_32.lib")

#define DEFAULT_BUFLEN 33
#define DEFAULT_PORT 2101
#define DEFAULT_IP "192.168.2.36"

// TODO: Reference additional headers your program requires here.
struct CNCVoltageMeter{
    std::string date;
    std::string time;
    std::string sourceAddress;
    int nodeId;
    int firmwareVersion;
    double batteryVoltage;
    int packetCounter;
    int sensorType;
    std::string channel1Name;
    double channel1Reading;
    std::string channel2Name;
    double channel2Reading;
};

static std::string app_path;
static std::stringstream logx;

int cnc_current_recorder(HANDLE*);

#endif