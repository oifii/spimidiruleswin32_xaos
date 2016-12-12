#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <iostream>
using std::cout; using std::endl;

#include <string>

#ifndef _SPIREGREADWRITE_H
#define _SPIREGREADWRITE_H

HKEY OpenKey(HKEY hRootKey, wchar_t* strKey);
void SetValDWORD(HKEY hKey, LPCTSTR lpValue, DWORD data);
DWORD GetValDWORD(HKEY hKey, LPCTSTR lpValue);

DWORD RegReadDWORD(HKEY hRootKey, wchar_t* strKey, LPCTSTR lpValue);
void RegWriteDWORD(HKEY hRootKey, wchar_t* strKey, LPCTSTR lpValue, DWORD data);

std::wstring RegReadSZ(HKEY hRootKey, wchar_t* strKey, LPCTSTR lpValue);
void RegWriteSZ(HKEY hRootKey, wchar_t* strKey, LPCTSTR lpValue, std::wstring data);

#endif //SPIREGREADWRITE_H