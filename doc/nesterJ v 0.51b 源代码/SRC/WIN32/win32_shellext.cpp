/*
** nester - NES emulator
** Copyright (C) 2000  Darren Ranalli
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful, 
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU 
** Library General Public License for more details.  To obtain a 
** copy of the GNU Library General Public License, write to the Free 
** Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
**
** Any permitted reproduction of these routines, in whole or in part,
** must bear this legend.
*/

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>


#include "win32_shellext.h"
#include "debug.h"

#define NESTER_ID "nesterROM"
#define NES_DESCRIPTION "NES ROM"

boolean AssociateNESExtension()
{
	HKEY dotNES_key = NULL;
	HKEY dotNES_defIcon_key = NULL;

	HKEY nesterID_key = NULL;
	HKEY nesterID_shell_key = NULL;
	HKEY nesterID_shell_open_key = NULL;
	HKEY nesterID_shell_open_command_key = NULL;

	char defIcon_str[MAX_PATH+128];

	char open_command[MAX_PATH];

	char full_exe_name[MAX_PATH];

	{
		// copy executable name from command line
		char* s = GetCommandLine();
		char* d = full_exe_name;

		// skip first "
		if(*s == '"') s++;

		// copy string
		while(*s && (*s != '"'))
			*(d++) = *(s++);

		// terminate the string
		*d = '\0';
	}

	try {

		// open the .NES key
		if(RegCreateKey(HKEY_CLASSES_ROOT, ".nes", &dotNES_key) != ERROR_SUCCESS)
			throw -1;

		// set the app ID string
		if(RegSetValue(dotNES_key, NULL, REG_SZ, NESTER_ID, strlen(NESTER_ID)) != ERROR_SUCCESS)
			throw -1;

		// set the default icon string
		//strcpy(defIcon_str, full_exe_name);
		//if(RegSetValue(dotNES_key, "DefaultIcon", REG_SZ, defIcon_str, strlen(defIcon_str)) != ERROR_SUCCESS)
		//	throw -1;


		// close the .NES key
		RegCloseKey(dotNES_key);
		dotNES_key = NULL;


		// open the nesterID key
		if(RegCreateKey(HKEY_CLASSES_ROOT, NESTER_ID, &nesterID_key) != ERROR_SUCCESS)
			throw -1;

		// set the document description string
		if(RegSetValue(nesterID_key, NULL, REG_SZ, NES_DESCRIPTION,
										strlen(NES_DESCRIPTION)) != ERROR_SUCCESS)
			throw -1;


		// open the DefaultIcon key
		if(RegCreateKey(nesterID_key, "DefaultIcon", &dotNES_defIcon_key) != ERROR_SUCCESS)
			throw -1;

		// set the default icon string
		//strcpy(defIcon_str, full_exe_name);
		sprintf( defIcon_str, "%s,%d",full_exe_name,1);
		if(RegSetValue(dotNES_defIcon_key, NULL, REG_SZ, defIcon_str, strlen(defIcon_str)) != ERROR_SUCCESS)
			throw -1;

		// close the DefaultIcon key
		RegCloseKey(dotNES_defIcon_key);
		dotNES_defIcon_key = NULL;


		// set the Open command

		// create the shell key
		if(RegCreateKey(nesterID_key, "Shell", &nesterID_shell_key) != ERROR_SUCCESS)
			throw -1;

		// create the open key
		if(RegCreateKey(nesterID_shell_key, "Open", &nesterID_shell_open_key) != ERROR_SUCCESS)
			throw -1;

		// create the command key
		if(RegCreateKey(nesterID_shell_open_key, "Command", &nesterID_shell_open_command_key) != ERROR_SUCCESS)
			throw -1;

		// create the open command string
		strcpy(open_command, full_exe_name);
		strcat(open_command, " \"%1\"");

		// set the open command
		if(RegSetValue(nesterID_shell_open_command_key, NULL, REG_SZ, open_command,
										strlen(open_command)) != ERROR_SUCCESS)
			throw -1;

		// close the command key
		RegCloseKey(nesterID_shell_open_command_key);
		nesterID_shell_open_command_key = NULL;

		// close the open key
		RegCloseKey(nesterID_shell_open_key);
		nesterID_shell_open_key = NULL;

		// close the shell key
		RegCloseKey(nesterID_shell_key);
		nesterID_shell_key = NULL;

		// close the nesterID key
		RegCloseKey(nesterID_key);
		nesterID_key = NULL;

	} catch(...) {
		if(dotNES_key)							RegCloseKey(dotNES_key);
		if(dotNES_defIcon_key)			RegCloseKey(dotNES_defIcon_key);
		if(nesterID_key)						RegCloseKey(nesterID_key);
		if(nesterID_shell_key)			RegCloseKey(nesterID_shell_key);
		if(nesterID_shell_open_key)	RegCloseKey(nesterID_shell_open_key);
		if(nesterID_shell_open_command_key)	RegCloseKey(nesterID_shell_open_command_key);
		return FALSE;
	}

	return TRUE;
}

void UndoAssociateNESExtension()
{
	// delete the .NES key
	RegDeleteKey(HKEY_CLASSES_ROOT, ".nes");

	// delete the nesterID key
	RegDeleteKey(HKEY_CLASSES_ROOT, NESTER_ID);
}
