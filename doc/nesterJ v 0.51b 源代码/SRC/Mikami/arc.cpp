/**
 * Ç›Ç©Ç›Ç©Ç»Ç…ÇÊÇÈägí£
 * GPL2 Ç…è]Ç¡ÇƒÇ≠ÇæÇ≥Ç¢
 */
#include <windows.h>
#include <shlwapi.h>
#include <stdio.h>
#include "arc.h"
#define M_ERROR_MESSAGE_OFF		0x00800000L

BOOL Uncompress( HWND hwnd, char **ptrlpBuf, LPCSTR ArcName, LPDWORD lpdwSize )
{
	const char *szLibNames[] = {
		"Unlha32",
		"UnZip32",
		"Unrar32",
		"Cab32",
		"Tar32",
		"Unarj32j",
		"Bga32"
	};

	const char *szFuncNamePreFixes[] = {
		"Unlha",
		"UnZip",
		"Unrar",
		"Cab",
		"Tar",
		"Unarj",
		"Bga"
	};
	
	// NULL is recommended in case DLL supporting "ExtractMem" function.
	const char *szCommands[] = {
		NULL,
		NULL,
		"-e -u \"%s\" \"%s\" \"%s\"",
		"-x -j \"%s\" \"%s\" \"%s\"",
		"-x -f \"%s\" -o \"%s\" \"%s\"",
		"e \"%s\" \"%s\" \"%s\"",
		"x -n \"%s\" \"%s\" \"%s\""
	};

	enum {
		LIB_UNLHA	= 0,
		LIB_UNZIP	= 1,
		LIB_UNRAR	= 2,
		LIB_CAB		= 3,
		LIB_TAR		= 4,
		LIB_UNARJ	= 5,
		LIB_BGA		= 6
	};


	const char *szExtensions[] = {
		"*.nes",
		"*.fds",
		"*.fam",
		"*.nsf"
	};

	int i,j,result;
	HMODULE hLib;
	HANDLE hf;
	HARC harc = NULL;
	char szFuncName[256];
	char szCommand[256];
	char szTempPath[512];
	char szTempFullName[512];
	EXECUTECOMMAND ExecuteCommand;
	CHECKARCHIVE CheckArchive;
	EXTRACTMEM ExtractMem;
	OPENARCHIVE OpenArchive;
	CLOSEARCHIVE CloseArchive;
	FINDFIRST FindFirst;
	INDIVIDUALINFO idvinfo;
	GetTempPath( MAX_PATH + 1, szTempPath );
	
	for( i=0; i<7; i++ )
	{
		if( !( hLib = GetModuleHandle( szLibNames[i] ) ) )
			continue;
		sprintf( szFuncName, "%sCheckArchive", szFuncNamePreFixes[i] );
		if( !(CheckArchive = (CHECKARCHIVE)GetProcAddress( hLib, szFuncName ) ) )
			continue;
		if( !CheckArchive( ArcName, 1 ) )
			continue;
		
		sprintf( szFuncName, "%sOpenArchive", szFuncNamePreFixes[i] );
		OpenArchive = (OPENARCHIVE)GetProcAddress( hLib, szFuncName );
		
		sprintf( szFuncName, "%sFindFirst", szFuncNamePreFixes[i] );
		FindFirst = (FINDFIRST)GetProcAddress( hLib, szFuncName );
		
		sprintf( szFuncName, "%sCloseArchive", szFuncNamePreFixes[i] );
		CloseArchive = (CLOSEARCHIVE)GetProcAddress( hLib, szFuncName );
		
		result = -1;
		for( j=0; j<4; j++ )
		{
			if( !(harc = OpenArchive( hwnd, ArcName, M_ERROR_MESSAGE_OFF ) ) )
			{	
				CloseArchive( harc );
				break;
			}
			result = FindFirst( harc, szExtensions[j], &idvinfo );
			CloseArchive( harc );
			if( result == 0 )
				break;
		}
		
		if( result )
			continue;

		if( szCommands[i] )
		{
			sprintf( szCommand, szCommands[i],
				ArcName, szTempPath, idvinfo.szFileName );
			ExecuteCommand = (EXECUTECOMMAND)GetProcAddress( hLib, szFuncNamePreFixes[i] );
			ExecuteCommand( hwnd, szCommand, NULL, 0 );
			
			sprintf( szTempFullName, "%s%s", szTempPath, idvinfo.szFileName );
			hf = CreateFile(
				szTempFullName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,
				FILE_FLAG_SEQUENTIAL_SCAN, NULL );
			if( hf == INVALID_HANDLE_VALUE )
				throw "ÉtÉ@ÉCÉãÇÃì«Ç›çûÇ›Ç…é∏îsÇµÇ‹ÇµÇΩ";
			*lpdwSize = GetFileSize( hf, NULL );
			*ptrlpBuf = (char*)malloc(*lpdwSize);
			DWORD rsize;
			ReadFile( hf, *ptrlpBuf, *lpdwSize, &rsize, NULL );
			CloseHandle( hf );
			DeleteFile( szTempFullName );
			if( *lpdwSize != rsize )
				throw "ÉtÉ@ÉCÉãÇÃì«Ç›çûÇ›Ç…é∏îsÇµÇ‹ÇµÇΩ";
			return TRUE;
		}
		else
		{
			char sz[FNAME_MAX32 + 1];
			if( i == LIB_UNZIP )
			{
				int p1=0, p2=0;
				while(true)
				{
					if( IsDBCSLeadByte( idvinfo.szFileName[p1] ) )
					{
						sz[p2++] = idvinfo.szFileName[p1++];
					}
					else switch( idvinfo.szFileName[p1] )
					{
					case '[':case ']':
						sz[p2++] = '\\';
					}
					if( !( sz[p2++] = idvinfo.szFileName[p1++] ) )
						break;
				}
			}
			else
				strcpy( sz, idvinfo.szFileName );
			*lpdwSize = idvinfo.dwOriginalSize;
			*ptrlpBuf = (char*)malloc(*lpdwSize);
			sprintf( szCommand, "\"%s\" \"%s\"", ArcName, sz );
			sprintf( szFuncName, "%sExtractMem", szFuncNamePreFixes[i] );
			ExtractMem = (EXTRACTMEM)GetProcAddress( hLib, szFuncName );
			result = ExtractMem(
						hwnd, szCommand, (LPBYTE)(*ptrlpBuf), *lpdwSize,
						NULL, NULL, NULL ); 
			if( result == 0 )
				return TRUE;
		}

	}
	return FALSE;
}
