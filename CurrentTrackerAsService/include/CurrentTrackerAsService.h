// CurrentTrackerAsService.h : Include file for standard system include files,
// or project specific include files.

#pragma once

#include "cnc_current_tracker.h"
#include "sql_connection.h"
#include <iostream>
#include <Windows.h>
#include <tchar.h>
#include <strsafe.h>
#include <cstdlib>


#define SERVICE_NAME LPTSTR("CNC_Current_Tracker")

SERVICE_STATUS	g_ServiceStatus = { 0 };
SERVICE_STATUS_HANDLE	g_StatusHandle = NULL;
HANDLE	g_ServiceStopEvent = INVALID_HANDLE_VALUE;

SC_HANDLE schSCManager;
SC_HANDLE schService;

VOID WINAPI ServiceMain(DWORD argc, LPTSTR* argv);
VOID WINAPI ServiceCtrlHandler(DWORD);
DWORD WINAPI ServiceWorkerThread(LPVOID lpParam);
VOID InstallService();
VOID __stdcall DeleteService();
VOID __stdcall StartService();
VOID __stdcall StopService();
BOOL __stdcall StopDependentServices();
