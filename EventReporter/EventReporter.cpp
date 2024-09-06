
#include "event_reporter.h"

//void ErrorExit()


LPCTSTR ErrorMessage(LPCTSTR lpszFunction)
{
	// Retrieve the system error message for the last-error code

	LPVOID lpMsgBuf;
	LPVOID lpDisplayBuf;
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
	lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT,
		(lstrlen((LPCTSTR)lpMsgBuf) + lstrlen((LPCTSTR)lpszFunction) + 40) * sizeof(TCHAR));
	StringCchPrintf((LPTSTR)lpDisplayBuf,
		LocalSize(lpDisplayBuf) / sizeof(TCHAR),
		TEXT("%s failed with error %d: %s"),
		lpszFunction, dw, lpMsgBuf);
	//MessageBox(NULL, (LPCTSTR)lpDisplayBuf, TEXT("Error"), MB_OK);
	LocalFree(lpMsgBuf);
	std::cout << (char*)lpDisplayBuf << std::endl;
	//LocalFree(lpDisplayBuf);
	//ExitProcess(dw);
	return (LPCTSTR)lpDisplayBuf;
}

LPCTSTR LogMessage(LPCTSTR lpszFunction, std::string Msg)
{
	// Retrieve the system error message for the last-error code

	LPVOID lpMsgBuf;
	LPVOID lpDisplayBuf;
	//DWORD dw = GetLastError();

	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_STRING |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		Msg.c_str(),
		0,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf,
		0, 
		NULL);

	// Display the error message and exit the process
	lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT,
		(lstrlen((LPCTSTR)Msg.c_str()) + lstrlen((LPCTSTR)lpszFunction) + 40) * sizeof(TCHAR));
	StringCchPrintf((LPTSTR)lpDisplayBuf,
		LocalSize(lpDisplayBuf) / sizeof(TCHAR),
		TEXT("%s triggered log event logging: %s"),
		lpszFunction, lpMsgBuf);
	//MessageBox(NULL, (LPCTSTR)lpDisplayBuf, TEXT("Error"), MB_OK);
	//LocalFree(lpMsgBuf);
	std::cout << (char*)lpDisplayBuf << std::endl;
	//LocalFree(lpDisplayBuf);
	//ExitProcess(dw);
	return (LPCTSTR)lpDisplayBuf;
}

VOID SvcReportEvent(LPCTSTR szFunction, std::string msg)
{
	HANDLE hEventSource;
	LPCTSTR lpszStrings[2];
	//TCHAR Buffer[80];
	HKEY hKey = OpenKey(HKEY_LOCAL_MACHINE, std::string("SOFTWARE\\CNC_Current_Tracker"));
	std::string SERVICE_NAME = GetStrVal(hKey, "ServiceName", REG_SZ);

	hEventSource = RegisterEventSource(NULL, SERVICE_NAME.c_str());

	if (NULL != hEventSource)
	{
		//StringCchPrintf(Buffer, 80, TEXT("%s failed with %d"), szFunction, GetLastError());

		lpszStrings[0] = SERVICE_NAME.c_str();
		lpszStrings[1] = msg == "" ? ErrorMessage(szFunction) : LogMessage(szFunction, msg);

		ReportEvent(
			hEventSource,        // event log handle
			msg == "" ? EVENTLOG_ERROR_TYPE : EVENTLOG_INFORMATION_TYPE, // event type
			0,                   // event category
			msg == "" ? SVC_ERROR : SVC_INFORMATION,           // event identifier
			NULL,                // no security identifier
			2,                   // size of lpszStrings array
			0,                   // no binary data
			lpszStrings,         // array of strings
			NULL);               // no binary data

		DeregisterEventSource(hEventSource);
	}
	RegCloseKey(hKey);
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
		reply.resize(buffSize-1);
		std::cout << "reply: " << reply << std::endl;

	}
	//getchar();
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