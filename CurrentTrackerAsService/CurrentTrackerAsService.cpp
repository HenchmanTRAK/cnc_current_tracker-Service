// CurrentTrackerAsService.cpp : Defines the entry point for the application.
//

#include "CurrentTrackerAsService.h"


//void ErrorExit()
void ErrorExit(LPCTSTR lpszFunction = "")
{
	// Retrieve the system error message for the last-error code

	LPVOID lpMsgBuf;
	//LPVOID lpDisplayBuf;
	DWORD dw = GetLastError();

	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		dw,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf,
		0, NULL);

	// Display the error message and exit the process

	/*lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT,
		(lstrlen((LPCTSTR)lpMsgBuf) + lstrlen((LPCTSTR)lpszFunction) + 40) * sizeof(TCHAR));
	StringCchPrintf((LPTSTR)lpDisplayBuf,
		LocalSize(lpDisplayBuf) / sizeof(TCHAR),
		TEXT("%s failed with error %d: %s"),
		lpszFunction, dw, lpMsgBuf);
	MessageBox(NULL, (LPCTSTR)lpDisplayBuf, TEXT("Error"), MB_OK);*/
	std::cout << "Error: " << dw << " " << (char*)lpMsgBuf << std::endl;
	LocalFree(lpMsgBuf);
	//LocalFree(lpDisplayBuf);
	//ExitProcess(dw);
}

int __cdecl _tmain(int argc, TCHAR *argv[])
{
	std::cout << SERVICE_NAME << *SERVICE_NAME << std::endl;
	HKEY hKey = OpenKey(HKEY_LOCAL_MACHINE, std::string("SYSTEM\\CurrentControlSet\\Services\\EventLog\\Application\\").append(SERVICE_NAME));
	std::string evtMsgFile = GetStrVal(hKey, "EventMessageFile", REG_SZ);
	GetVal(hKey, "TypesSupported", REG_DWORD);
	std::cout << evtMsgFile << std::endl;
	char buff[1024];
	int byteLength;
	std::string currDir;
	if (evtMsgFile == "") {
		byteLength = GetCurrentDirectory(sizeof(buff), buff);
		std::string currDir = buff;
		currDir.resize(byteLength);
		evtMsgFile = currDir;
		//+ "\\sample.dll";

	}
	std::cout << evtMsgFile << std::endl;
	SetStrVal(hKey, "EventMessageFile", evtMsgFile, REG_SZ);
	SetVal(hKey, "TypesSupported", 7, REG_DWORD);
	RegCloseKey(hKey);

	hKey = OpenKey(HKEY_LOCAL_MACHINE, std::string("SOFTWARE\\").append(SERVICE_NAME));
	std::string installPath = GetStrVal(hKey, "InstallPath", REG_SZ);
	if (installPath == "") {
		byteLength = GetCurrentDirectory(sizeof(buff), buff);
		std::string currDir = buff;
		currDir.resize(byteLength);
		installPath = currDir;
	}
	SetStrVal(hKey, "InstallPath", installPath, REG_SZ);

	std::string iniFilePath = GetStrVal(hKey, "IniFilePath", REG_SZ);
	if (iniFilePath == "") {
		byteLength = GetCurrentDirectory(sizeof(buff), buff);
		std::string currDir = buff;
		currDir.resize(byteLength);
		iniFilePath = currDir + "\\settings.ini";
	}
	SetStrVal(hKey, "IniFilePath", iniFilePath, REG_SZ);

	CSimpleIni ini;
	ini.SetUnicode();
	iniFilePath = GetStrVal(hKey, "IniFilePath", REG_SZ);

	SI_Error rc = ini.LoadFile(iniFilePath.c_str());
	if (rc < 0) {
		std::cerr << "Failed to Load INI File" << std::endl;
		getchar();
		return 1;
	}

	CSimpleIniA::TNamesDepend keys;
	ini.GetAllKeys("Database", keys);
	for (auto it = keys.begin(); it != keys.end(); ++it) {
		std::cout << std::string(it->pItem) << std::endl;
		std::string value(ini.GetValue("Database", it->pItem, ""));
		value.erase(
			remove(value.begin(), value.end(), '\"'),
			value.end()
		);
		std::cout << it->pItem << "=" << value << std::endl;
		SetStrVal(hKey, it->pItem, value ,REG_SZ);
	}

	RegCloseKey(hKey);
	//printf(_T("CNC_Current_Tracker_Service: Main: Entry"));
	/*std::cout << lstrcmpi(argv[1], TEXT("install")) << std::endl;
	getchar();*/

	/*GetPrivateProfileString("Database", "Host", NULL, database_host, sizeof(database_host) / sizeof(database_host[0]), ".\\settings.ini");
	GetPrivateProfileString("Database", "DSN", NULL, database_DSN, sizeof(database_DSN) / sizeof(database_DSN[0]), ".\\settings.ini");
	GetPrivateProfileString("Database", "Schema", NULL, database_schema, sizeof(database_schema) / sizeof(database_schema[0]), ".\\settings.ini");
	GetPrivateProfileString("Database", "Username", NULL, database_user, sizeof(database_user) / sizeof(database_user[0]), ".\\settings.ini");
	GetPrivateProfileString("Database", "Password", NULL, database_pass, sizeof(database_pass) / sizeof(database_pass[0]), ".\\settings.ini");*/

	if (lstrcmpi(argv[1], TEXT("install")) == 0 || lstrcmpi(argv[1], TEXT("install")) == -1)
	{

		DoInstallSrv();
		//return 0;
		/*StartTheService();
		getchar();*/
		DoStartSvc();
	}

	if (lstrcmpi(argv[1], TEXT("remove")) == 0)
	{
		DoStopSvc();
		DoDeleteSvc();
		/*StopService();
		getchar();*/
		LONG nError = RegDeleteKey(HKEY_LOCAL_MACHINE, std::string("SYSTEM\\CurrentControlSet\\Services\\EventLog\\Application\\").append(SERVICE_NAME).data());
	}

	SERVICE_TABLE_ENTRY ServiceTable[] =
	{
		{(LPSTR)SERVICE_NAME, (LPSERVICE_MAIN_FUNCTION) ServiceMain},
		{NULL, NULL}
	};

	if (!StartServiceCtrlDispatcher(ServiceTable))
	{
		//SvcReportEvent(TEXT("StartServiceCtrlDispatcher"));
		ErrorExit(TEXT("GetProcessId"));
	}
	printf(TEXT("CNC_Current_Tracker_Service: Main: StartServiceCtrlDispatcher"));
	getchar();
	return 0;
}

HKEY OpenKey(HKEY hRootKey, std::string strKey) {
	HKEY hKey;
	//std::string strKey = "SYSTEM\\CurrentControlSet\\Services\\EventLog\\Application\\" + SERVICE_NAME;
	std::cout << strKey << std::endl;
	LONG nError = RegOpenKeyEx(hRootKey, strKey.data(), NULL, KEY_ALL_ACCESS, &hKey);
	if (nError == ERROR_FILE_NOT_FOUND)
	{
		std::cout << "Creating registry key: " << strKey << std::endl;
		nError = RegCreateKeyEx(hRootKey, strKey.data(), NULL, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey, NULL);
	}

	if (nError)
		std::cout << "Error: " << nError << " Could not find or create " << strKey << std::endl;

	return hKey;
}

void SetStrVal(HKEY hKey, LPCTSTR lpValue, std::string data, DWORD type)
{
	LONG nError = RegSetValueEx(hKey, lpValue, NULL, type, (LPBYTE)data.c_str(), data.size() + 1);

	if (nError)
		std::cout << "Error: " << nError << " Could not set registry value: " << (char*)lpValue << std::endl;
}

void SetVal(HKEY hKey, LPCTSTR lpValue, DWORD data, DWORD type)
{
	LONG nError = RegSetValueEx(hKey, lpValue, NULL, type, (LPBYTE)&data, sizeof(data));

	if (nError)
		std::cout << "Error: " << nError << " Could not set registry value: " << (char*)lpValue << std::endl;
}

std::string GetStrVal(HKEY hKey, LPCTSTR lpValue, DWORD type)
{
	DWORD buffSize = 1024;
	char data[1024];
	std::string reply;
	std::cout << lpValue << std::endl;
	//LONG nError = RegQueryValueEx(hKey, lpValue, NULL, &type, (LPBYTE)data, &buffSize);
	LONG nError = RegGetValue(hKey, NULL, lpValue, RRF_RT_ANY, NULL, data, &buffSize);

	if (nError == ERROR_FILE_NOT_FOUND) {
		std::cout << "No File Found" << std::endl;
		reply = ""; // The value will be created and set to data next time SetVal() is called.
	}
	else if (nError)
		std::cout << "Error: " << nError << " Could not get registry value " << (char*)lpValue << std::endl;
	else {
		std::cout << "data: " << data << bool(nError == ERROR_FILE_NOT_FOUND) << std::endl;
		reply = data;
		reply.resize(buffSize);
		std::cout << "reply: " << reply << std::endl;

	}
	getchar();
	return reply;
}

DWORD GetVal(HKEY hKey, LPCTSTR lpValue, DWORD type)
{
	DWORD data;		DWORD size = sizeof(data);
	LONG nError = RegQueryValueEx(hKey, lpValue, NULL, &type, (LPBYTE)&data, &size);

	if (nError == ERROR_FILE_NOT_FOUND)
		data = 0; // The value will be created and set to data next time SetVal() is called.
	else if (nError)
		std::cout << "Error: " << nError << " Could not get registry value " << (char*)lpValue << std::endl;

	return data;
}

VOID DoInstallSrv()
{
	/*SC_HANDLE schSCManager;
    SC_HANDLE schService;*/
	TCHAR szUnquotedPath[MAX_PATH];

	if (!GetModuleFileName(NULL, szUnquotedPath, MAX_PATH))
	{
		printf("Cannot install service (%d)\n", GetLastError());
		ErrorExit(TEXT("GetProcessId"));
		return;
	}

	// In case the path contains a space, it must be quoted so that
	// it is correctly interpreted. For example,
	// "d:\my share\myservice.exe" should be specified as
	// ""d:\my share\myservice.exe"".
	TCHAR szPath[MAX_PATH];
	StringCbPrintf(szPath, MAX_PATH, TEXT("\"%s\""), szUnquotedPath);
	//std::cout << szPath << "|" << szUnquotedPath << std::endl;
	// Get a handle to the SCM database. 

	schSCManager = OpenSCManager(
		NULL,                    // local computer
		NULL,                    // ServicesActive database 
		SC_MANAGER_ALL_ACCESS);  // full access rights 

	if (NULL == schSCManager)
	{
		printf("OpenSCManager failed (%d)\n", GetLastError());
		ErrorExit(TEXT("GetProcessId"));
		return;
	}

	// Create the service
	// | SERVICE_INTERACTIVE_PROCESS
	schService = CreateService(
		schSCManager,					// SCM database 
		SERVICE_NAME,					// name of service 
		SERVICE_NAME,					// service name to display 
		SERVICE_ALL_ACCESS,				// desired access 
		SERVICE_WIN32_OWN_PROCESS,		// service type 
		SERVICE_AUTO_START,				// start type 
		SERVICE_ERROR_NORMAL,			// error control type 
		szPath,							// path to service's binary 
		NULL,							// no load ordering group 
		NULL,							// no tag identifier 
		NULL,							// no dependencies 
		NULL,							// LocalSystem account 
		NULL							// no password 
	);                     

	if (schService == NULL)
	{
		printf("CreateService failed (%d)\n", GetLastError());
		ErrorExit(TEXT("GetProcessId"));
		CloseServiceHandle(schSCManager);
		return;
	}
	else printf("Service installed successfully\n");

	CloseServiceHandle(schService);
	CloseServiceHandle(schSCManager);
}

VOID __stdcall DoDeleteSvc()
{
	/*SC_HANDLE schSCManager;
	SC_HANDLE schService;*/
	SERVICE_STATUS ssStatus;

	// Get a handle to the SCM database. 
	schSCManager = OpenSCManager(
		NULL,                    // local computer
		NULL,                    // ServicesActive database 
		SC_MANAGER_ALL_ACCESS);  // full access rights 

	if (NULL == schSCManager)
	{
		printf("OpenSCManager failed (%d)\n", GetLastError());
		ErrorExit(TEXT("GetProcessId"));
		return;
	}

	// Get a handle to the service.

	schService = OpenService(
		schSCManager,			// SCM database 
		SERVICE_NAME,			// name of service 
		DELETE);				// need delete access 

	if (schService == NULL)
	{
		printf("OpenService failed (%d)\n", GetLastError());
		ErrorExit(TEXT("GetProcessId"));
		CloseServiceHandle(schSCManager);
		return;
	}

	// Delete the service.

	if (!DeleteService(schService))
	{
		printf("DeleteService failed (%d)\n", GetLastError());
		ErrorExit(TEXT("GetProcessId"));
	}
	else printf("Service deleted successfully\n");

	CloseServiceHandle(schService);
	CloseServiceHandle(schSCManager);
}

VOID __stdcall DoStartSvc()
{
	SERVICE_STATUS_PROCESS ssStatus;
	DWORD dwOldCheckPoint;
	DWORD dwStartTickCount;
	DWORD dwWaitTime;
	DWORD dwBytesNeeded;

	// Get a handle to the SCM database. 

	schSCManager = OpenSCManager(
		NULL,                    // local computer
		NULL,                    // servicesActive database 
		SC_MANAGER_ALL_ACCESS);  // full access rights 

	if (NULL == schSCManager)
	{
		printf("OpenSCManager failed (%d)\n", GetLastError());
		ErrorExit(TEXT("GetProcessId"));
		return;
	}

	// Get a handle to the service.

	schService = OpenService(
		schSCManager,			// SCM database 
		SERVICE_NAME,			// name of service 
		SERVICE_ALL_ACCESS);	// full access 

	if (schService == NULL)
	{
		printf("OpenService failed (%d)\n", GetLastError());
		ErrorExit(TEXT("GetProcessId"));
		CloseServiceHandle(schSCManager);
		return;
	}

	// Check the status in case the service is not stopped. 
	if (!QueryServiceStatusEx(
		schService,                     // handle to service 
		SC_STATUS_PROCESS_INFO,         // information level
		(LPBYTE)&ssStatus,             // address of structure
		sizeof(SERVICE_STATUS_PROCESS), // size of structure
		&dwBytesNeeded))              // size needed if buffer is too small
	{
		printf("QueryServiceStatusEx failed (%d)\n", GetLastError());
		ErrorExit(TEXT("GetProcessId"));
		CloseServiceHandle(schService);
		CloseServiceHandle(schSCManager);
		return;
	}

	// Check if the service is already running. It would be possible 
	// to stop the service here, but for simplicity this example just returns. 

	if (ssStatus.dwCurrentState != SERVICE_STOPPED && ssStatus.dwCurrentState != SERVICE_STOP_PENDING)
	{
		printf("Cannot start the service because it is already running\n");
		CloseServiceHandle(schService);
		CloseServiceHandle(schSCManager);
		return;
	}

	// Save the tick count and initial checkpoint.

	dwStartTickCount = GetTickCount();
	dwOldCheckPoint = ssStatus.dwCheckPoint;

	// Wait for the service to stop before attempting to start it.

	while (ssStatus.dwCurrentState == SERVICE_STOP_PENDING)
	{
		// Do not wait longer than the wait hint. A good interval is 
		// one-tenth of the wait hint but not less than 1 second  
		// and not more than 10 seconds. 

		dwWaitTime = ssStatus.dwWaitHint / 10;

		if (dwWaitTime < 1000)
			dwWaitTime = 1000;
		else if (dwWaitTime > 10000)
			dwWaitTime = 10000;

		Sleep(dwWaitTime);

		// Check the status until the service is no longer stop pending. 

		if (!QueryServiceStatusEx(
			schService,                     // handle to service 
			SC_STATUS_PROCESS_INFO,         // information level
			(LPBYTE)&ssStatus,             // address of structure
			sizeof(SERVICE_STATUS_PROCESS), // size of structure
			&dwBytesNeeded))              // size needed if buffer is too small
		{
			printf("QueryServiceStatusEx failed (%d)\n", GetLastError());
			ErrorExit(TEXT("GetProcessId"));
			CloseServiceHandle(schService);
			CloseServiceHandle(schSCManager);
			return;
		}

		if (ssStatus.dwCheckPoint > dwOldCheckPoint)
		{
			// Continue to wait and check.

			dwStartTickCount = GetTickCount();
			dwOldCheckPoint = ssStatus.dwCheckPoint;
		}
		else
		{
			if (GetTickCount() - dwStartTickCount > ssStatus.dwWaitHint)
			{
				printf("Timeout waiting for service to stop\n");
				CloseServiceHandle(schService);
				CloseServiceHandle(schSCManager);
				return;
			}
		}
	}

	// Attempt to start the service.

	if (!StartService(
		schService,	// handle to service 
		0,			// number of arguments 
		NULL))		// no arguments 
	{
		printf("StartService failed (%d)\n", GetLastError());
		ErrorExit(TEXT("GetProcessId"));
		CloseServiceHandle(schService);
		CloseServiceHandle(schSCManager);
		DoStopSvc();
		DoDeleteSvc();
		return;
	}
	else printf("Service start pending...\n");

	// Check the status until the service is no longer start pending. 

	if (!QueryServiceStatusEx(
		schService,                     // handle to service 
		SC_STATUS_PROCESS_INFO,         // info level
		(LPBYTE)&ssStatus,             // address of structure
		sizeof(SERVICE_STATUS_PROCESS), // size of structure
		&dwBytesNeeded))              // if buffer too small
	{
		printf("QueryServiceStatusEx failed (%d)\n", GetLastError());
		ErrorExit(TEXT("GetProcessId"));
		CloseServiceHandle(schService);
		CloseServiceHandle(schSCManager);
		return;
	}

	// Save the tick count and initial checkpoint.

	dwStartTickCount = GetTickCount();
	dwOldCheckPoint = ssStatus.dwCheckPoint;

	while (ssStatus.dwCurrentState == SERVICE_START_PENDING)
	{
		// Do not wait longer than the wait hint. A good interval is 
		// one-tenth the wait hint, but no less than 1 second and no 
		// more than 10 seconds. 

		dwWaitTime = ssStatus.dwWaitHint / 10;

		if (dwWaitTime < 1000)
			dwWaitTime = 1000;
		else if (dwWaitTime > 10000)
			dwWaitTime = 10000;

		Sleep(dwWaitTime);

		// Check the status again. 

		if (!QueryServiceStatusEx(
			schService,             // handle to service 
			SC_STATUS_PROCESS_INFO, // info level
			(LPBYTE)&ssStatus,             // address of structure
			sizeof(SERVICE_STATUS_PROCESS), // size of structure
			&dwBytesNeeded))              // if buffer too small
		{
			printf("QueryServiceStatusEx failed (%d)\n", GetLastError());
			ErrorExit(TEXT("GetProcessId"));
			break;
		}

		if (ssStatus.dwCheckPoint > dwOldCheckPoint)
		{
			// Continue to wait and check.

			dwStartTickCount = GetTickCount();
			dwOldCheckPoint = ssStatus.dwCheckPoint;
		}
		else
		{
			if (GetTickCount() - dwStartTickCount > ssStatus.dwWaitHint)
			{
				// No progress made within the wait hint.
				break;
			}
		}
	}

	// Determine whether the service is running.

	if (ssStatus.dwCurrentState == SERVICE_RUNNING)
	{
		printf("Service started successfully.\n");
	}
	else
	{
		printf("Service not started. \n");
		printf("  Current State: %d\n", ssStatus.dwCurrentState);
		printf("  Exit Code: %d\n", ssStatus.dwWin32ExitCode);
		printf("  Check Point: %d\n", ssStatus.dwCheckPoint);
		printf("  Wait Hint: %d\n", ssStatus.dwWaitHint);
		DoDeleteSvc();
	}

	CloseServiceHandle(schService);
	CloseServiceHandle(schSCManager);
}

VOID __stdcall DoStopSvc()
{


	SERVICE_STATUS_PROCESS ssp;
	DWORD dwStartTime = GetTickCount();
	DWORD dwBytesNeeded;
	DWORD dwTimeout = 30000; // 30-second time-out
	DWORD dwWaitTime;

	
	// Get a handle to the SCM database. 
	schSCManager = OpenSCManager(
		NULL,                    // local computer
		NULL,                    // ServicesActive database 
		SC_MANAGER_ALL_ACCESS);  // full access rights 

	if (NULL == schSCManager)
	{
		printf("OpenSCManager failed (%d)\n", GetLastError());
		ErrorExit(TEXT("GetProcessId"));
		return;
	}

	// Get a handle to the service.

	schService = OpenService(
		schSCManager,			// SCM database 
		SERVICE_NAME,	// name of service 
		SERVICE_STOP |
		SERVICE_QUERY_STATUS |
		SERVICE_ENUMERATE_DEPENDENTS);

	if (schService == NULL)
	{
		printf("OpenService failed (%d)\n", GetLastError());
		ErrorExit(TEXT("GetProcessId"));
		CloseServiceHandle(schSCManager);
		return;
	}

	// Make sure the service is not already stopped.

	if (!QueryServiceStatusEx(
		schService,
		SC_STATUS_PROCESS_INFO,
		(LPBYTE)&ssp,
		sizeof(SERVICE_STATUS_PROCESS),
		&dwBytesNeeded))
	{
		printf("QueryServiceStatusEx failed (%d)\n", GetLastError());
		ErrorExit(TEXT("GetProcessId"));
		goto stop_cleanup;
	}

	if (ssp.dwCurrentState == SERVICE_STOPPED)
	{
		printf("Service is already stopped.\n");
		goto stop_cleanup;
	}

	// If a stop is pending, wait for it.

	while (ssp.dwCurrentState == SERVICE_STOP_PENDING)
	{
		printf("Service stop pending...\n");

		// Do not wait longer than the wait hint. A good interval is 
		// one-tenth of the wait hint but not less than 1 second  
		// and not more than 10 seconds. 

		dwWaitTime = ssp.dwWaitHint / 10;

		if (dwWaitTime < 1000)
			dwWaitTime = 1000;
		else if (dwWaitTime > 10000)
			dwWaitTime = 10000;

		Sleep(dwWaitTime);

		if (!QueryServiceStatusEx(
			schService,
			SC_STATUS_PROCESS_INFO,
			(LPBYTE)&ssp,
			sizeof(SERVICE_STATUS_PROCESS),
			&dwBytesNeeded))
		{
			printf("QueryServiceStatusEx failed (%d)\n", GetLastError());
			ErrorExit(TEXT("GetProcessId"));
			goto stop_cleanup;
		}

		if (ssp.dwCurrentState == SERVICE_STOPPED)
		{
			printf("Service stopped successfully.\n");
			goto stop_cleanup;
		}

		if (GetTickCount() - dwStartTime > dwTimeout)
		{
			printf("Service stop timed out.\n");
			goto stop_cleanup;
		}
	}

	// If the service is running, dependencies must be stopped first.

	StopDependentServices();

	// Send a stop code to the service.

	if (!ControlService(
		schService,
		SERVICE_CONTROL_STOP,
		(LPSERVICE_STATUS)&ssp))
	{
		printf("ControlService failed (%d)\n", GetLastError());
		ErrorExit(TEXT("GetProcessId"));
		goto stop_cleanup;
	}

	// Wait for the service to stop.

	while (ssp.dwCurrentState != SERVICE_STOPPED)
	{
		Sleep(ssp.dwWaitHint/10);
		if (!QueryServiceStatusEx(
			schService,
			SC_STATUS_PROCESS_INFO,
			(LPBYTE)&ssp,
			sizeof(SERVICE_STATUS_PROCESS),
			&dwBytesNeeded))
		{
			printf("QueryServiceStatusEx failed (%d)\n", GetLastError());
			ErrorExit(TEXT("GetProcessId"));
			goto stop_cleanup;
		}

		if (ssp.dwCurrentState == SERVICE_STOPPED)
			break;

		if (GetTickCount() - dwStartTime > dwTimeout)
		{
			printf("Wait timed out\n");
			goto stop_cleanup;
		}
	}
	printf("Service stopped successfully\n");

stop_cleanup:
	CloseServiceHandle(schService);
	CloseServiceHandle(schSCManager);
}

BOOL __stdcall StopDependentServices()
{
	DWORD i;
	DWORD dwBytesNeeded;
	DWORD dwCount;

	LPENUM_SERVICE_STATUS   lpDependencies = NULL;
	ENUM_SERVICE_STATUS     ess;
	SC_HANDLE               hDepService;
	SERVICE_STATUS_PROCESS  ssp;
	

	DWORD dwStartTime = GetTickCount();
	DWORD dwTimeout = 30000; // 30-second time-out

	// Pass a zero-length buffer to get the required buffer size.
	if (EnumDependentServices(schService, SERVICE_ACTIVE,
		lpDependencies, 0, &dwBytesNeeded, &dwCount))
	{
		// If the Enum call succeeds, then there are no dependent
		// services, so do nothing.
		return TRUE;
	}
	else
	{
		if (GetLastError() != ERROR_MORE_DATA)
			return FALSE; // Unexpected error

		// Allocate a buffer for the dependencies.
		lpDependencies = (LPENUM_SERVICE_STATUS)HeapAlloc(
			GetProcessHeap(), HEAP_ZERO_MEMORY, dwBytesNeeded);

		if (!lpDependencies)
			return FALSE;

		__try {
			// Enumerate the dependencies.
			if (!EnumDependentServices(schService, SERVICE_ACTIVE,
				lpDependencies, dwBytesNeeded, &dwBytesNeeded,
				&dwCount))
				return FALSE;

			for (i = 0; i < dwCount; i++)
			{
				ess = *(lpDependencies + i);
				// Open the service.
				hDepService = OpenService(schSCManager,
					ess.lpServiceName,
					SERVICE_STOP | SERVICE_QUERY_STATUS);

				if (!hDepService)
					return FALSE;

				__try {
					// Send a stop code.
					if (!ControlService(hDepService,
						SERVICE_CONTROL_STOP,
						(LPSERVICE_STATUS)&ssp))
						return FALSE;

					// Wait for the service to stop.
					while (ssp.dwCurrentState != SERVICE_STOPPED)
					{
						Sleep(ssp.dwWaitHint);
						if (!QueryServiceStatusEx(
							hDepService,
							SC_STATUS_PROCESS_INFO,
							(LPBYTE)&ssp,
							sizeof(SERVICE_STATUS_PROCESS),
							&dwBytesNeeded))
							return FALSE;

						if (ssp.dwCurrentState == SERVICE_STOPPED)
							break;

						if (GetTickCount() - dwStartTime > dwTimeout)
							return FALSE;
					}
				}
				__finally
				{
					// Always release the service handle.
					CloseServiceHandle(hDepService);
				}
			}
		}
		__finally
		{
			// Always free the enumeration buffer.
			HeapFree(GetProcessHeap(), 0, lpDependencies);
		}
	}
	return TRUE;
}

VOID WINAPI ServiceMain(DWORD argc, LPTSTR* argv)
{	
	DWORD Status = E_FAIL;


	// Registering Service Controller
	printf("Registering Service Controler");
	g_StatusHandle = RegisterServiceCtrlHandler(
		SERVICE_NAME,
		ServiceCtrlHandler
	);

	if (!g_StatusHandle || g_StatusHandle == NULL)
	{
		//SvcReportEvent(TEXT("RegisterServiceCtrlHandler"));
		return;
	}

	printf("Defining service type and target exit code");
	ZeroMemory(&g_ServiceStatus, sizeof(g_ServiceStatus));
	g_ServiceStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
	g_ServiceStatus.dwServiceSpecificExitCode = 0;
	g_ServiceStatus.dwControlsAccepted = 0;
	g_ServiceStatus.dwCurrentState = SERVICE_START_PENDING;
	g_ServiceStatus.dwWin32ExitCode = 0;
	g_ServiceStatus.dwCheckPoint = 0;
	//ReportSvcStatus(SERVICE_START_PENDING, NO_ERROR, 3000);

	if (SetServiceStatus(g_StatusHandle, &g_ServiceStatus) == FALSE)
		printf(_T("CNC_Current_Tracker_Service: ServiceMain: SetServiceStatus returned error"));

	// starting the service
	
	printf("Initializing service");
	g_ServiceStopEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
	if (g_ServiceStopEvent == NULL)
	{
		//ReportSvcStatus(SERVICE_STOPPED, GetLastError(), 0);
		ErrorExit(TEXT("GetProcessId"));
		g_ServiceStatus.dwControlsAccepted = 0;
		g_ServiceStatus.dwCurrentState = SERVICE_STOPPED;
		g_ServiceStatus.dwWin32ExitCode = GetLastError();
		g_ServiceStatus.dwCheckPoint = 1;
		if (SetServiceStatus(g_StatusHandle, &g_ServiceStatus) == FALSE)
			printf(_T(
				"My Sample Service: ServiceMain: SetServiceStatus returned error"));
		//printf(_T("CNC_Current_Tracker_Service: ServiceInit: Exit"));
		return;
	}
	//ReportSvcStatus(SERVICE_START_PENDING, NO_ERROR, 3000);
	printf("creating service worker thread");
	g_ServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;
	g_ServiceStatus.dwCurrentState = SERVICE_RUNNING;
	g_ServiceStatus.dwWin32ExitCode = 0;
	g_ServiceStatus.dwCheckPoint = 0;

	if (SetServiceStatus(g_StatusHandle, &g_ServiceStatus) == FALSE) 
		printf(_T("CNC_Current_Tracker_Service: ServiceMain: SetServiceStatus returned error"));

	HANDLE hThread = CreateThread(NULL, 0, ServiceWorkerThread, NULL, 0, NULL);

	if (hThread) WaitForSingleObject(g_ServiceStopEvent, INFINITE);
	
	/*if (hThread) {
		printf("Waiting for Thread Handler");
		ReportSvcStatus(SERVICE_STOPPED, NO_ERROR, 0);
	}*/
	
	CloseHandle(g_ServiceStopEvent);

	g_ServiceStatus.dwControlsAccepted = 0;
	g_ServiceStatus.dwCurrentState = SERVICE_STOPPED;
	g_ServiceStatus.dwWin32ExitCode = 0;
	g_ServiceStatus.dwCheckPoint = 3;

	if (SetServiceStatus(g_StatusHandle, &g_ServiceStatus) == FALSE) printf(_T("CNC_Current_Tracker_Service: ServiceMain: SetServiceStatus returned error"));

	printf(_T("CNC_Current_Tracker_Service: ServiceMain: Exit"));
	return;
}

VOID WINAPI ServiceCtrlHandler(DWORD CtrlCode)
{
	switch(CtrlCode)
	{
	case SERVICE_CONTROL_STOP:
	{
		//ReportSvcStatus(SERVICE_STOP_PENDING, NO_ERROR, 0);
		if (g_ServiceStatus.dwCurrentState != SERVICE_RUNNING) break;

		g_ServiceStatus.dwControlsAccepted = 0;
		g_ServiceStatus.dwCurrentState = SERVICE_STOP_PENDING;
		g_ServiceStatus.dwWin32ExitCode = 0;
		g_ServiceStatus.dwCheckPoint = 4;

		if (SetServiceStatus(g_StatusHandle, &g_ServiceStatus) == FALSE) 
			printf(_T("CNC_Current_Tracker_Service: ServiceCtrlHandler: SetServiceStatus returned error"));
		SetEvent(g_ServiceStopEvent);
		//ReportSvcStatus(g_ServiceStatus.dwCurrentState, NO_ERROR, 0);
		break;
	}
	default:
	{
		break;
	}
	}
}

DWORD WINAPI ServiceWorkerThread(LPVOID lpParam)
{
	//ReportSvcStatus(SERVICE_RUNNING, NO_ERROR, 0);
	while (WaitForSingleObject(g_ServiceStopEvent, 0) != WAIT_OBJECT_0)
	{
		cnc_current_recorder();
	}

	return ERROR_SUCCESS;
}

//VOID ReportSvcStatus(DWORD dwCurrentState,
//	DWORD dwWin32ExitCode,
//	DWORD dwWaitHint)
//{
//	static DWORD dwCheckPoint = 1;
//
//	// Fill in the SERVICE_STATUS structure.
//
//	g_ServiceStatus.dwCurrentState = dwCurrentState;
//	g_ServiceStatus.dwWin32ExitCode = dwWin32ExitCode;
//	g_ServiceStatus.dwWaitHint = dwWaitHint;
//
//	if (dwCurrentState == SERVICE_START_PENDING)
//		g_ServiceStatus.dwControlsAccepted = 0;
//	else g_ServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;
//
//	if ((dwCurrentState == SERVICE_RUNNING) ||
//		(dwCurrentState == SERVICE_STOPPED))
//		g_ServiceStatus.dwCheckPoint = 0;
//	else g_ServiceStatus.dwCheckPoint = dwCheckPoint++;
//
//	// Report the status of the service to the SCM.
//	SetServiceStatus(g_StatusHandle, &g_ServiceStatus);
//}
//
//VOID SvcReportEvent(const char *szFunction)
//{
//	HANDLE hEventSource;
//	LPCTSTR lpszStrings[2];
//	TCHAR Buffer[80];
//
//	hEventSource = RegisterEventSource(NULL, SERVICE_NAME);
//
//	if (NULL != hEventSource)
//	{
//		StringCchPrintf(Buffer, 80, TEXT("%s failed with %d"), szFunction, GetLastError());
//
//		lpszStrings[0] = SERVICE_NAME;
//		lpszStrings[1] = Buffer;
//
//		ReportEvent(hEventSource,        // event log handle
//			EVENTLOG_ERROR_TYPE, // event type
//			0,                   // event category
//			SVC_ERROR,           // event identifier
//			NULL,                // no security identifier
//			2,                   // size of lpszStrings array
//			0,                   // no binary data
//			lpszStrings,         // array of strings
//			NULL);               // no binary data
//
//		DeregisterEventSource(hEventSource);
//	}
//}