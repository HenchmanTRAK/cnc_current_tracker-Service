#ifndef EVENT_REPORTER_H
#define EVENT_REPORTER_H 1
#pragma once


#include "sample.h"
#include <stdlib.h>
#include <iostream>
#include <string>
#include <Windows.h>
#include <strsafe.h>


LPCTSTR ErrorMessage(LPCTSTR lpszFunction);
LPCTSTR LogMessage(LPCTSTR lpszFunction, std::string Msg);
VOID SvcReportEvent(LPCTSTR szFunction, std::string msg = "");
HKEY OpenKey(HKEY, std::string);
void SetStrVal(HKEY hKey, LPCTSTR lpValue, std::string data, DWORD type);
void SetVal(HKEY, LPCTSTR, DWORD, DWORD);
std::string GetStrVal(HKEY, LPCTSTR, DWORD);
DWORD GetVal(HKEY, LPCTSTR, DWORD);

#endif