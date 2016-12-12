
#include "spiregreadwrite.h"


HKEY OpenKey(HKEY hRootKey, wchar_t* strKey)
{
	HKEY hKey;
	LONG nError = RegOpenKeyEx(hRootKey, strKey, NULL, KEY_ALL_ACCESS, &hKey);

	if (nError==ERROR_FILE_NOT_FOUND)
	{
		//cout << "Creating registry key: " << strKey << endl;
		nError = RegCreateKeyEx(hRootKey, strKey, NULL, NULL, REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,NULL, &hKey, NULL);
	}

	if (nError)
		cout << "Error: " << nError << " Could not find or create " << strKey << endl;

	return hKey;
}

void SetValDWORD(HKEY hKey, LPCTSTR lpValue, DWORD data)
{
	LONG nError = RegSetValueEx(hKey, lpValue, NULL, REG_DWORD, (LPBYTE)&data, sizeof(DWORD));

	if (nError)
		cout << "Error: " << nError << " Could not set registry value: " << (char*)lpValue << endl;
}

DWORD GetValDWORD(HKEY hKey, LPCTSTR lpValue)
{
	DWORD data;		DWORD size = sizeof(data);	DWORD type = REG_DWORD;
	LONG nError = RegQueryValueEx(hKey, lpValue, NULL, &type, (LPBYTE)&data, &size);

	if (nError==ERROR_FILE_NOT_FOUND)
		data = 0; // The value will be created and set to data next time SetVal() is called.
	else if (nError)
		cout << "Error: " << nError << " Could not get registry value " << (char*)lpValue << endl;

	return data;
}

DWORD RegReadDWORD(HKEY hRootKey, wchar_t* strKey, LPCTSTR lpValue)
{
	DWORD value;
	HKEY hKey = OpenKey(hRootKey, strKey);
	value = GetValDWORD(hKey, lpValue);
	RegCloseKey(hKey);
	return value;
}

void RegWriteDWORD(HKEY hRootKey, wchar_t* strKey, LPCTSTR lpValue, DWORD data)
{
	DWORD value;
	HKEY hKey = OpenKey(hRootKey, strKey);
	SetValDWORD(hKey, lpValue, data);
	RegCloseKey(hKey);
}

//see: http://stackoverflow.com/questions/10675705/how-can-i-read-a-key-value-of-registry-and-print-it-to-the-screen-with-the-mess
std::wstring RegReadSZ(HKEY hRootKey, wchar_t* strKey, LPCTSTR lpValue)
{
	HKEY hKey = OpenKey(hRootKey, strKey);

    DWORD type;
    DWORD cbData;
    if (RegQueryValueEx(hKey, lpValue, NULL, &type, NULL, &cbData) != ERROR_SUCCESS)
    {
        RegCloseKey(hKey);
        throw "Could not read registry value";
    }

    if (type != REG_SZ)
    {
        RegCloseKey(hKey);
        throw "Incorrect registry value type";
    }

    std::wstring value(cbData/sizeof(wchar_t), L'\0');
    if (RegQueryValueEx(hKey, lpValue, NULL, NULL, reinterpret_cast<LPBYTE>(&value[0]), &cbData) != ERROR_SUCCESS)
    {
        RegCloseKey(hKey);
        throw "Could not read registry value";
    }

	RegCloseKey(hKey);

    size_t firstNull = value.find_first_of(L'\0');
    if (firstNull != std::string::npos)
        value.resize(firstNull);

	return value;
}

void RegWriteSZ(HKEY hRootKey, wchar_t* strKey, LPCTSTR lpValue, std::wstring data)
{
	HKEY hKey = OpenKey(hRootKey, strKey);

	LONG nError = RegSetValueEx(hKey, lpValue, NULL, REG_SZ, (LPBYTE)(data.c_str()), (2*data.length())+1);
	if (nError)
		cout << "Error: " << nError << " Could not set registry value: " << (char*)lpValue << endl;
	RegCloseKey(hKey);
}


