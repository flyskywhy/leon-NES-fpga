#include <windows.h>
#include <shlwapi.h>
#include <stdlib.h>
#include "mkutils.h"
#include "settings.h"

void MKCreateDirectories( const char *path )
{
	if( !(*path) )
		return;
	char p1[MAX_PATH];
	char *p = p1;
	strcpy( p1, path ); 
	PathAddBackslash( p1 );
	
	while( p = StrChr( p, '\\' ) )
	{
		*p = '\0';
		CreateDirectory( p1, NULL );
		*p = '\\';
		p++;
	}
}

int MKGetLongFileName( char *longPath, const char *sourcePath )
{
	char *filePart;
	GetFullPathName( sourcePath, MAX_PATH, longPath, &filePart );
	WIN32_FIND_DATA fd;
	HANDLE hff = FindFirstFile( longPath, &fd );
	if( hff == INVALID_HANDLE_VALUE )
	{
		longPath[0] = '\0';
		return 0;
	}
	FindClose( hff );
	strcpy( filePart, fd.cFileName );
	return strlen( longPath );
}

