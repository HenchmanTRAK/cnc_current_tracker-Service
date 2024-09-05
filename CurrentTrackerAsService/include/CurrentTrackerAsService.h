// CurrentTrackerAsService.h : Include file for standard system include files,
// or project specific include files.

#pragma once

#include "cnc_current_recorder.h"
#include "sql_connection.h"
//#include "sample.h"
#include <iostream>
#include <Windows.h>
#include <tchar.h>
#include <strsafe.h>
#include <cstdlib>
#include <algorithm>
#include <TlHelp32.h>

#include "SimpleIni.h"

#pragma comment(lib, "advapi32.lib")

#define SERVICE_NAME "CNC_Current_Tracker"

SERVICE_STATUS		  g_ServiceStatus		= { 0 };
SERVICE_STATUS_HANDLE g_StatusHandle		= NULL;
HANDLE				  g_ServiceStopEvent	= INVALID_HANDLE_VALUE;

//static std::string database_host;
//static std::string database_DSN;
//static std::string database_schema;
//static std::string database_user;
//static std::string database_pass;
//static std::string database_port;

SC_HANDLE schSCManager;
SC_HANDLE schService;

VOID WINAPI ServiceMain(DWORD argc, LPTSTR* argv);
VOID ServiceInit(DWORD argc, LPTSTR* argv);
VOID WINAPI ServiceCtrlHandler(DWORD);
DWORD WINAPI ServiceWorkerThread(LPVOID lpParam);
VOID WINAPI DoInstallSrv();
VOID WINAPI __stdcall DoDeleteSvc();
VOID WINAPI __stdcall DoStartSvc();
VOID WINAPI __stdcall DoStopSvc();
BOOL WINAPI __stdcall StopDependentServices();
HKEY OpenKey(HKEY , std::string);
void SetStrVal(HKEY hKey, LPCTSTR lpValue, std::string data, DWORD type);
void SetVal(HKEY , LPCTSTR, DWORD, DWORD);
std::string GetStrVal(HKEY , LPCTSTR , DWORD );
DWORD GetVal(HKEY , LPCTSTR, DWORD);
//VOID ReportSvcStatus(DWORD, DWORD, DWORD);
//VOID SvcReportEvent(const char*);
