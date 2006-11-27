#ifndef CSAVEDIRS_H_
#define CSAVEDIRS_H_

#include <windows.h>
#include <shlwapi.h>

class CPathSettings
{
public:
	char szSramPath[MAX_PATH];
	char szStatePath[MAX_PATH];
    char szShotPath[MAX_PATH];
	char szWavePath[MAX_PATH];
	char szLastStatePath[MAX_PATH];
	char szAppPath[MAX_PATH];
	bool UseSramPath;
	bool UseStatePath;
	bool UseShotPath;
	bool UseWavePath;
	CPathSettings()
	{
		HKEY hKey;
		DWORD dwDisposition, cbSize, dwType = 0;
		GetModuleFileName( NULL, szAppPath, MAX_PATH );
		PathRemoveFileSpec( szAppPath );
		PathAddBackslash( szAppPath );
		int result = RegCreateKeyEx(
			HKEY_CURRENT_USER, "software\\nesterJ\\Paths",  0,
			NULL, REG_OPTION_NON_VOLATILE, KEY_READ, NULL, &hKey, &dwDisposition );
		
		if( result == ERROR_SUCCESS )
		{
#define LoadValue(_NAME_,_LPDATA_) \
			RegQueryValueEx(hKey,_NAME_,NULL,&dwType,_LPDATA_,&cbSize)
			
			cbSize = MAX_PATH;
			if( ( LoadValue( "SRAM", (BYTE*)szSramPath ) != ERROR_SUCCESS )
				|| ( dwType != REG_SZ ) )
			{
				strcpy( szSramPath, szAppPath );
				strcat( szSramPath, "save\\" );
			}
			
			cbSize = MAX_PATH;
			if( ( LoadValue( "State", (BYTE*)szStatePath ) != ERROR_SUCCESS )
				|| ( dwType != REG_SZ ) )
			{
				strcpy( szStatePath, szAppPath );
				strcat( szStatePath, "state\\" );
			}

			cbSize = MAX_PATH;
			if( ( LoadValue( "Shot", (BYTE*)szShotPath ) != ERROR_SUCCESS )
				|| ( dwType != REG_SZ ) )
			{
				strcpy( szShotPath, szAppPath );
				strcat( szShotPath, "shot\\" );
			}
			
			cbSize = MAX_PATH;
			if( ( LoadValue( "Wave", (BYTE*)szWavePath ) != ERROR_SUCCESS )
				|| ( dwType != REG_SZ ) )
			{
				strcpy( szWavePath, szAppPath );
				strcat( szWavePath, "wave\\" );
			}
			
            cbSize = MAX_PATH;
			if( ( LoadValue( "LastState", (BYTE*)szLastStatePath ) != ERROR_SUCCESS )
				|| ( dwType != REG_SZ ) )
			{
				strcpy( szLastStatePath, szAppPath );
			}
			
			cbSize = sizeof(bool);
			if( ( LoadValue( "UseSramPath", (BYTE*)&UseSramPath ) != ERROR_SUCCESS )
				|| ( dwType != REG_BINARY ) )
				UseSramPath = true;
			
			cbSize = sizeof(bool);
			if( ( LoadValue( "UseStatePath", (BYTE*)&UseStatePath ) != ERROR_SUCCESS )
				|| ( dwType != REG_BINARY ) )
				UseStatePath = false;
			
			cbSize = sizeof(bool);
			if( ( LoadValue( "UseShotPath", (BYTE*)&UseShotPath ) != ERROR_SUCCESS )
				|| ( dwType != REG_BINARY ) )
				UseShotPath = true;
			
			cbSize = sizeof(bool);
			if( ( LoadValue( "UseWavePath", (BYTE*)&UseWavePath ) != ERROR_SUCCESS )
				|| ( dwType != REG_BINARY ) )
				UseWavePath = true;
#undef LoadValue
		}
		RegCloseKey( hKey );
	}
	
	~CPathSettings()
	{
		HKEY hKey;
		DWORD dwDisposition;
		RegCreateKeyEx(
			HKEY_CURRENT_USER, "software\\nesterJ\\Paths",  0,
			NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, &dwDisposition );

#define SaveValue(_NAME_,_TYPE_,_LPDATA_,_SIZE_) \
		RegSetValueEx(hKey,_NAME_,0,_TYPE_,_LPDATA_,_SIZE_)
		SaveValue( "SRAM", REG_SZ, (BYTE*)szSramPath, strlen(szSramPath) );
		SaveValue( "State", REG_SZ, (BYTE*)szStatePath, strlen(szStatePath) );
		SaveValue( "Shot", REG_SZ, (BYTE*)szShotPath, strlen(szShotPath) );
		SaveValue( "Wave", REG_SZ, (BYTE*)szWavePath, strlen(szWavePath) );
		SaveValue( "LastState", REG_SZ, (BYTE*)szLastStatePath, strlen(szLastStatePath) );
		SaveValue( "UseSramPath", REG_BINARY, (BYTE*)&UseSramPath, sizeof(bool) );
		SaveValue( "UseStatePath", REG_BINARY, (BYTE*)&UseStatePath, sizeof(bool) );
		SaveValue( "UseShotPath", REG_BINARY, (BYTE*)&UseShotPath, sizeof(bool) );
		SaveValue( "UseWavePath", REG_BINARY, (BYTE*)&UseWavePath, sizeof(bool) );

#undef SaveValue
		RegCloseKey( hKey );

	}

};
#endif