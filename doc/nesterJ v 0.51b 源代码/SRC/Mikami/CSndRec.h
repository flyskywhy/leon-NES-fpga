/**
 * みかみかなによる拡張。GPL2 に従ってください。
 * このクラスは継承されることを想定していません。
 */

#ifndef CSNDREC_H_
#define CSNDREC_H_

#include <stdio.h>
#include <windows.h>

#pragma pack(1)
typedef unsigned short	word;
typedef unsigned long	dword;
typedef struct
{
	char		typeFile[4];
	dword		sizeFile;
	char		typeSub[8];
	dword		offsetFirstTag;
	word		typeCoding;
	word		nChannel;
	dword		nSample;
	dword		nBytesPerSec;
	word		nBytesPerSample;
	word		nBitrate;
	char		tagFirst[4];
	dword		sizeData;
} RIFFHEADER, *LPRIFFHEADER;
#pragma pack()

class CSndRec
{
public:
	CSndRec( const char *szFileName, dword nSampleBits, dword nSampleRate );
	~CSndRec();
	void write( void *lpBuf, dword size );

private:
	RIFFHEADER WaveHead;
	FILE *fwav;
};

#endif
