#ifndef ARC_H_
#define ARC_H_
#include <TIME.H>
#pragma pack(1)
#define FNAME_MAX32 512

typedef	HGLOBAL	HARC;
typedef struct {
	DWORD 			dwOriginalSize;		/* ファイルのサイズ */
 	DWORD 			dwCompressedSize;	/* 圧縮後のサイズ */
	DWORD			dwCRC;				/* 格納ファイルのチェックサム */
	UINT			uFlag;				/* 処理結果 */
	UINT			uOSType;			/* 書庫作成に使われたＯＳ */
	WORD			wRatio;				/* 圧縮率 */
	WORD			wDate;				/* 格納ファイルの日付(DOS 形式) */
	WORD 			wTime;				/* 格納ファイルの時刻(〃) */
	char			szFileName[FNAME_MAX32 + 1];	/* 書庫名 */
	char			dummy1[3];
	char			szAttribute[8];		/* 格納ファイルの属性(書庫固有) */
	char			szMode[8];			/* 格納ファイルの格納モード(〃) */
}	INDIVIDUALINFO, *LPINDIVIDUALINFO;

typedef struct {
	DWORD 			dwFileSize;		/* 格納ファイルのサイズ */
	DWORD			dwWriteSize;	/* 書き込みサイズ */
	char			szSourceFileName[FNAME_MAX32 + 1];	/* 格納ファイル名 */
	char			dummy1[3];
	char			szDestFileName[FNAME_MAX32 + 1];
									/* 解凍先または圧縮元パス名 */
	char			dummy[3];
}	EXTRACTINGINFO, *LPEXTRACTINGINFO;

typedef struct {
	EXTRACTINGINFO exinfo;
	DWORD dwCompressedSize;
	DWORD dwCRC;
	UINT  uOSType;
	WORD  wRatio;
	WORD  wDate;
	WORD  wTime;
	char  szAttribute[8];
	char  szMode[8];
} EXTRACTINGINFOEX, *LPEXTRACTINGINFOEX;

typedef struct {
	DWORD			dwStructSize;
	UINT			uCommand;
	DWORD			dwOriginalSize;
	DWORD			dwCompressedSize;
	DWORD			dwAttributes;
	DWORD			dwCRC;
	UINT			uOSType;
	WORD			wRatio;
	FILETIME		ftCreateTime;
	FILETIME		ftAccessTime;
	FILETIME		ftWriteTime;
	char			szFileName[FNAME_MAX32 + 1];
	char			dummy1[3];
	char			szAddFileName[FNAME_MAX32 + 1];
	char			dummy2[3];
}	UNLHA_ENUM_MEMBER_INFO, *LPUNLHA_ENUM_MEMBER_INFO;

typedef BOOL (CALLBACK *UNLHA_WND_ENUMMEMBPROC)(LPUNLHA_ENUM_MEMBER_INFO);

#pragma pack()

typedef int(WINAPI *EXECUTECOMMAND)(HWND,LPCSTR,LPSTR,const DWORD);
typedef BOOL(WINAPI *CHECKARCHIVE)(LPCSTR,const int);
typedef int(WINAPI *EXTRACTMEM)(HWND,LPCSTR,LPBYTE,const DWORD,time_t,LPWORD,LPDWORD);
typedef HARC(WINAPI *OPENARCHIVE)(HWND,LPCSTR,const DWORD);
typedef int(WINAPI *CLOSEARCHIVE)(HARC);
typedef int(WINAPI *FINDFIRST)(HARC,LPCSTR,INDIVIDUALINFO*);
BOOL Uncompress( HWND hwnd, char **lpBuf, LPCSTR ArcName, LPDWORD lpdwSize );

#endif