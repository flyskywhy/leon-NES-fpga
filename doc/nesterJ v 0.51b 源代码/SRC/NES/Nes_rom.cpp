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
** 
** 圧縮ファイル対応のため、みかみかな によって、Win32 APIの File IO に
** 書きかえられています。
**
** 武田によってFAM,FDS形式の読み込みに関するコードが追加されています。
*/

#include <windows.h>
#include <shlwapi.h>
#include "NES_ROM.h"
#include "arc.h"
#include "debug.h"


#define CopyMemIncPtr(o,i,s) \
{\
	memcpy(o,i,s);\
	i+=s;\
}

enum {
	NES_HEADER_SIZE = 16,
};


NES_ROM::NES_ROM(const char* fn)
{
	HANDLE hf = NULL;
	char *buf = NULL;
	char *p = NULL;
	
	trainer    = NULL;
	ROM_banks  = NULL;
	VROM_banks = NULL;
	
	rom_name = NULL;
	rom_path = NULL;
	uint8 image_type = 0;

	try 
	{
		// store filename and path
		rom_name = (char*)malloc(strlen(fn)+1);
		rom_path = (char*)malloc(strlen(fn)+1);
		if( !rom_name || !rom_path)
			throw "Error loading file: out of memory";

		GetPathInfo( fn );

		hf = CreateFile(
				fn, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,
				FILE_FLAG_SEQUENTIAL_SCAN, NULL );
		if( hf == INVALID_HANDLE_VALUE ) throw "Error opening file";
		
		int filesize, readsize;
		filesize = GetFileSize( hf, NULL );
		if( filesize == -1 ) throw "Error opening file";
		if( NES_HEADER_SIZE > filesize )
			throw "Error reading from file";
		if( !( buf = p = (char*)malloc(filesize) ) )
			throw "Error loading file: out of memory";
		if( !ReadFile( hf, p, NES_HEADER_SIZE, (DWORD*)&readsize, NULL ) ||
			( readsize != NES_HEADER_SIZE ) )
			throw "Error reading from file";
		CopyMemIncPtr( &header, p, NES_HEADER_SIZE );
		if( ( !strncmp( (char*)header.id, "NES", 3) && ( header.ctrl_z == 0x1A) ) ||
			( !strncmp( (char*)header.id, "NEZ", 3) && ( header.ctrl_z == 0x1A) ) ||
			( !strncmp( (char*)header.id, "FDS", 3) && ( header.ctrl_z == 0x1A) ) ||
			( header.id[0] <= 0x1A && header.id[1] == 0x00 && header.num_8k_vrom_banks == 0x00 ) ||
			( !strncmp( (char*)header.id, "NES", 3) && (header.ctrl_z == 'M'  ) )
		  )
		{
			if( !ReadFile( hf, p, filesize-NES_HEADER_SIZE, (DWORD*)&readsize, NULL) ||
				( ( readsize += NES_HEADER_SIZE ) != filesize ) )
				throw "Error Reading from file";
			CloseHandle( hf );
		}
		else
		{
			CloseHandle( hf );
			free(buf);
			buf = NULL;
			if( !Uncompress( NULL, &buf, fn, (LPDWORD)&filesize ) )
#if defined(_NESTERJ_ENGLISH)
				throw "Unsupported File";
#else
				throw "対応していないファイルです";
#endif
			p = buf;
			if( p+NES_HEADER_SIZE-buf > filesize )
#if defined(_NESTERJ_ENGLISH)
				throw "Error reading Header";
#else
				throw "ヘッダの読み込みに失敗しました";
#endif
			CopyMemIncPtr( &header, p, NES_HEADER_SIZE );
		}
		header.num_16k_rom_banks = (!header.dummy) ? 256 : header.dummy;
		
		if( !strncmp( (const char*)header.id, "NES", 3) && ( header.ctrl_z == 0x1A) ||
			!strncmp( (const char*)header.id, "NEZ", 3) && ( header.ctrl_z == 0x1A) )
		{
			// allocate memory
			ROM_banks = (uint8*)malloc(header.num_16k_rom_banks * (16*1024));
			if( !ROM_banks ) throw "Out of memory";
			
			VROM_banks = (uint8*)malloc(header.num_8k_vrom_banks * (8*1024));
			if( !VROM_banks ) throw "Out of memory";
				
			// load trainer if present
			if( has_trainer() )
			{
				if( !( trainer = (uint8*)malloc(TRAINER_LEN) ) )
					throw "Out of memory";
				if( p+TRAINER_LEN-buf > filesize )
#if defined(_NESTERJ_ENGLISH)
					throw "Error reading Trainer";
#else
					throw "Trainer の読み込みに失敗しました";
#endif
				CopyMemIncPtr( trainer, p, TRAINER_LEN );
			}

			if( p + (16*1024) * header.num_16k_rom_banks - buf > filesize )
#if defined(_NESTERJ_ENGLISH)
				throw "Error reading ROM banks";
#else
				throw "ROM バンクの読み込みに失敗しました";
#endif
			CopyMemIncPtr( ROM_banks, p, (16*1024) * header.num_16k_rom_banks );

			if( p + (8*1024) * header.num_8k_vrom_banks - buf > filesize )
#if defined(_NESTERJ_ENGLISH)
				throw "Error reading VROM banks";
#else
				throw "VROM バンクの読み込みに失敗しました";
#endif
			CopyMemIncPtr( VROM_banks, p, (8*1024) * header.num_8k_vrom_banks );
			if(((header.flags_1 >> 4) | (header.flags_2 & 0xF0)) == 20)
			{
				// convert NES disk image
				uint32 rom_pt, disk_pt, i, j, k;
				uint8 file_num, disk_num;
				uint8 disk[0x40000];
				uint8 disk_header[15] = {
					0x01,0x2A,0x4E,0x49,0x4E,0x54,0x45,0x4E,0x44,0x4F,0x2D,0x48,0x56,0x43,0x2A
				};
				
				disk_num = header.num_16k_rom_banks >> 2;
				disk_num = (disk_num > 4) ? 4 : disk_num;

				for (i = 0; i < disk_num; i++)
				{
					file_num = ROM_banks[0x10000*i+0x3F];
					rom_pt = 0x10000*i+0x40;
					disk_pt = 0x10000*i+0x3A;
					for(j = 0x00; j <= 0x0E; j++)
					{
						disk[0x10000*i+j] = disk_header[j];
					}
					for(j = 0x0F; j <= 0x37; j++)
					{
						disk[0x10000*i+j] = ROM_banks[0x10000*i+j-0x0F];
					}
					disk[0x10000*i+0x38] = 0x02;
					disk[0x10000*i+0x39] = file_num;
					for(j = 0; j < file_num; j++)
					{
						if(disk_pt < 0x10000*(i+1))
						{
							uint32 file_size = ROM_banks[rom_pt+13]+ROM_banks[rom_pt+14]*256;
							for(k = 0; k < 16; k++)
							{
								disk[disk_pt++] = ROM_banks[rom_pt++];
							}
							disk[disk_pt++] = 0x04;
							for(k = 0; k < file_size; k++)
							{
								disk[disk_pt++] = ROM_banks[rom_pt++];
							}
						}
					}
				}
				ROM_banks[0] = 'F';
				ROM_banks[1] = 'D';
				ROM_banks[2] = 'S';
				ROM_banks[3] = 0x1A;
				ROM_banks[4] = disk_num;
				for(i = 0; i < disk_num; i++)
				{
					for(uint32 j = 0; j < 65500; j++)
					{
						ROM_banks[65500*i+j+16] = disk[0x10000*i+j];
					}
				}
			}
		}
		else if( !strncmp( (const char*)header.id, "FDS", 3) && ( header.ctrl_z == 0x1A) )
		{
			image_type = 1;
			uint8 disk_num = header.num_16k_rom_banks;

			header.id[0] = 'N';
			header.id[1] = 'E';
			header.id[2] = 'S';
			header.num_16k_rom_banks *= 4;
			header.num_8k_vrom_banks = 0;
			header.flags_1 = 0x40;
			header.flags_2 = 0x10;
			
			header.reserved[0] = header.reserved[1] = header.reserved[2] = header.reserved[3] = 0;
			header.reserved[4] = header.reserved[5] = header.reserved[6] = header.reserved[7] = 0;

			// allocate memory
			ROM_banks = (uint8*)malloc(16+65500*disk_num);
			if(!ROM_banks) throw "Out of memory";

			VROM_banks = (uint8*)malloc(0);
			if(!VROM_banks) throw "Out of memory";

			if( p + 65500 * disk_num - buf > filesize )
#if defined(_NESTERJ_ENGLISH)
				throw "Error reading FDS Image";
#else
				throw "FDS イメージの読み込みに失敗しました";
#endif
			CopyMemIncPtr( ROM_banks + 16, p, 65500 * disk_num );

			ROM_banks[0] = 'F';
			ROM_banks[1] = 'D';
			ROM_banks[2] = 'S';
			ROM_banks[3] = 0x1A;
			ROM_banks[4] = disk_num;
		}
		else if(header.id[0] <= 0x1A && header.id[1] == 0x00 && header.num_8k_vrom_banks == 0x00)
		{
			image_type = 1;
			uint8 fam[6];
			fam[0] = header.id[0];
			fam[1] = header.id[1];
			fam[2] = header.id[2];
			fam[3] = header.ctrl_z;
			fam[4] = (uint8)header.num_16k_rom_banks;
			fam[5] = header.num_8k_vrom_banks;

			p = 6 + buf;

			while(!((fam[0] == 0x13 || fam[0] == 0x1A) && fam[1] == 0x00))
			{
				if(p + (uint32)fam[2]+((uint32)fam[3]<<8)+((uint32)fam[4]<<16)-6 - buf > filesize)
#if defined(_NESTERJ_ENGLISH)
					throw "Error reading FAM image";
#else
					throw "FAM イメージの読み込みに失敗しました";
#endif
				p += (uint32)fam[2]+((uint32)fam[3]<<8)+((uint32)fam[4]<<16)-6;
				if(p + 6 - buf > filesize)
#if defined(_NESTERJ_ENGLISH)
					throw "Error reading FAM image";
#else
					throw "FAM イメージの読み込みに失敗しました";
#endif
				CopyMemIncPtr( fam, p, 6 );
			}

			uint8 disk_num = (uint8)(((uint32)fam[2]+((uint32)fam[3]<<8)+((uint32)fam[4]<<16))/65500);

			header.id[0] = 'N';
			header.id[1] = 'E';
			header.id[2] = 'S';
			header.num_16k_rom_banks = disk_num*4;
			header.num_8k_vrom_banks = 0;
			header.flags_1 = 0x40;
			header.flags_2 = 0x10;
			header.reserved[0] = header.reserved[1] = header.reserved[2] = header.reserved[3] = 0;
			header.reserved[4] = header.reserved[5] = header.reserved[6] = header.reserved[7] = 0;

			// allocate memory
			ROM_banks = (uint8*)malloc(16+65500*disk_num);
			if(!ROM_banks) throw "Out of memory";

			VROM_banks = (uint8*)malloc(0);
			if(!VROM_banks) throw "Out of memory";
			
			if(fam[0] == 0x1A)
			{
				if( p + 16 - buf > filesize )
#if defined(_NESTERJ_ENGLISH)
					throw "Error reading FAM image";
#else
					throw "FAM イメージの読み込みに失敗しました";
#endif
				p += 16;
			}
			
			if( p + 65500 * disk_num - buf > filesize )
#if defined(_NESTERJ_ENGLISH)
					throw "Error reading FAM image";
#else
					throw "FAM イメージの読み込みに失敗しました";
#endif
			CopyMemIncPtr( ROM_banks + 16, p, 65500 * disk_num );

			ROM_banks[0] = 'F';
			ROM_banks[1] = 'D';
			ROM_banks[2] = 'S';
			ROM_banks[3] = 0x1A;
			ROM_banks[4] = disk_num;
		}
		else if(!strncmp((const char*)header.id, "NES", 3) && (header.ctrl_z == 'M'))
		{
			image_type = 2;
			ROM_banks = (uint8*)malloc(0x40000);
			if(!ROM_banks) throw "Out of memory";

			VROM_banks = (uint8*)malloc(0);
			if(!VROM_banks) throw "Out of memory";
			
			uint32 i = 0x10;
			
			if( filesize > 0x40000 )
#if defined(_NESTERJ_ENGLISH)
				throw "NSF file is over 256 KB";
#else
				throw "256 KBを超えたNSFファイルには対応していません";
#endif

			CopyMemIncPtr( ROM_banks + 0x10, p, filesize - ( p - buf ) );
			
			ROM_banks[0x0] = (i >>  0) & 0xFF;
			ROM_banks[0x1] = (i >>  8) & 0xFF;
			ROM_banks[0x2] = (i >> 16) & 0xFF;
			ROM_banks[0x3] = (i >> 24) & 0xFF;
			ROM_banks[0x4] = (uint8)header.num_16k_rom_banks;
			ROM_banks[0x5] = header.num_8k_vrom_banks;
			ROM_banks[0x6] = header.flags_1;
			ROM_banks[0x7] = header.flags_2;
			ROM_banks[0x8] = header.reserved[0];
			ROM_banks[0x9] = header.reserved[1];
			ROM_banks[0xA] = header.reserved[2];
			ROM_banks[0xB] = header.reserved[3];
			ROM_banks[0xC] = header.reserved[4];
			ROM_banks[0xD] = header.reserved[5];
			ROM_banks[0xE] = header.reserved[6];
			ROM_banks[0xF] = header.reserved[7];

			header.id[0] = 'N';
			header.id[1] = 'E';
			header.id[2] = 'S';
			header.ctrl_z = 0x1A;
			header.num_16k_rom_banks = 1;
			header.num_8k_vrom_banks = 0;
			header.flags_1 = 0xC0;
			header.flags_2 = 0x00;
			header.reserved[0] = header.reserved[1] = header.reserved[2] = header.reserved[3] = 0;
			header.reserved[4] = header.reserved[5] = header.reserved[6] = header.reserved[7] = 0;
		}
		else
		{
			throw "Unsupported File";
		}
		free(buf);
	}
	catch(...)
	{
		CloseHandle(hf);
		if(buf)			free(buf);
		if(VROM_banks)	free(VROM_banks);
		if(ROM_banks)	free(ROM_banks);
		if(trainer)		free(trainer);
		if(rom_name)	free(rom_name);
		if(rom_path)	free(rom_path);
		throw;
	}

	uint32 i,j;
	crc = fds = 0;
	screen_mode = 0;

	if(image_type == 1)
	{
		screen_mode = 1;
		mapper = 20;

		fds = (ROM_banks[0x1f] << 24) | (ROM_banks[0x20] << 16) |
			(ROM_banks[0x21] <<  8) | (ROM_banks[0x22] <<  0);

	    for(i = 0; i < ROM_banks[4]; i++)
	    {
			uint8 file_num = 0;
			uint32 pt = 16+65500*i+0x3a;
			while(ROM_banks[pt] == 0x03)
			{
				pt += 0x0d;
				pt += ROM_banks[pt] + ROM_banks[pt+1] * 256 + 4;
				file_num++;
			}
			ROM_banks[16+65500*i+0x39] = file_num;
		}
	}
	else if(image_type == 2)
	{
		screen_mode = 1;
		mapper = 12; // 12 is private mapper number
	}
	else
	{
		unsigned long c, crctable[256];

		for(i = 0; i < 256; i++)
		{
			c = i;
			for (j = 0; j < 8; j++)
			{
				if (c & 1)
					c = (c >> 1) ^ 0xedb88320;
				else
					c >>= 1;
			}
			crctable[i] = c;
		}

		for(i = 0; i < header.num_16k_rom_banks; i++)
		{
			c = ~crc;
			for(j = 0; j < 0x4000; j++)
				c = crctable[(c ^ ROM_banks[i*0x4000+j]) & 0xff] ^ (c >> 8);
			crc = ~c;
		}

		// read nestoy database
		{
			FILE* fp2;
			char fn2[256], buf[256];

			GetModuleFileName(NULL, fn2, 256);
			int pt = strlen(fn2);
			while(fn2[pt] != '\\') pt--;
			fn2[pt+1] = '\0';
			strcat(fn2, "nesdbase.dat");

			if((fp2 = fopen(fn2, "r")) != NULL)
			{
				while(fgets(buf, 256, fp2))
				{
					pt = 0;

					// All CRC
					while(buf[pt] != ';') pt++;
					pt++;

					// PROM CRC
					c = 0;
					for(i = 0; i < 8; i++)
					{
						if('0' <= buf[pt] && buf[pt] <= '9')
							c = ((c & 0x0fffffff) << 4) | (buf[pt] - '0');
						else if('A' <= buf[pt] && buf[pt] <= 'F')
							c = ((c & 0x0fffffff) << 4) | (buf[pt] - 'A' + 10);
						else if('a' <= buf[pt] && buf[pt] <= 'f')
							c = ((c & 0x0fffffff) << 4) | (buf[pt] - 'a' + 10);
						pt++;
					}
					while(buf[pt] != ';') pt++;
					pt++;

					if(crc == c && crc != 0)
					{
						char buf2[16];

						// Title
						while(buf[pt] != ';') pt++;
						pt++;

						// Header 1
						i = 0;
						while(buf[pt] != ';') buf2[i++] = buf[pt++];
						pt++;
						buf2[i] = '\0';
						header.flags_1 = atoi(buf2);

						// Header 2
						i = 0;
						while(buf[pt] != ';') buf2[i++] = buf[pt++];
						pt++;
						buf2[i] = '\0';
						header.flags_2 = atoi(buf2);

						// PROM Size
						while(buf[pt] != ';') pt++;
						pt++;

						// CROM Size
						while(buf[pt] != ';') pt++;
						pt++;

						// Country
						if( buf[pt] == 'J' || ( buf[pt] == 'P' && buf[pt+1] != 'D') || buf[pt] == 'U' || buf[pt] == 'V')
						{
							// Japan, PlayChoice-10, US, VS Unisystem
							screen_mode = 1;
						}
						else if(buf[pt] == 'A' || buf[pt] == 'E' || (buf[pt] == 'P' && buf[pt+1] == 'D') || buf[pt] == 'S')
						{
							// Asia, Europe, PD, Swedish
							screen_mode = 2;
						}

						for(i = 0; i < 8; i++) header.reserved[i] = 0;
						fseek(fp2, 0, SEEK_END);
					}
				}
				fclose(fp2);
			}
		}
	}
	// figure out mapper number
	mapper = ( header.flags_1 >> 4);
	
	// if there is anything in the reserved bytes,
	// don't trust the high nybble of the mapper number
	if( ((header.flags_2 & 0x0F) > 0x02) || ((header.flags_2 >= 0x10) && (header.reserved[0] != 0)) )
		header.flags_2= 0;
	mapper |= ( header.flags_2 & 0xF0 );
	
	#include "NES_rom_Correct.cpp"
	
}

NES_ROM::~NES_ROM()
{
	if(VROM_banks)	free(VROM_banks);
	if(ROM_banks)	free(ROM_banks);
	if(trainer)		free(trainer);
	if(rom_name)	free(rom_name);
	if(rom_path)	free(rom_path);
}

void NES_ROM::GetPathInfo(const char* fn)
{
	strcpy( rom_path, fn );
	PathRemoveFileSpec( rom_path );
	PathAddBackslash( rom_path );

	strcpy( rom_name, PathFindFileName( fn ) );
	PathRemoveExtension( rom_name );
}

