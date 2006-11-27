#ifndef ARC_H_
#define ARC_H_
#include <TIME.H>
#pragma pack(1)
#define FNAME_MAX32 512

typedef	HGLOBAL	HARC;
typedef struct {
	DWORD 			dwOriginalSize;		/* �t�@�C���̃T�C�Y */
 	DWORD 			dwCompressedSize;	/* ���k��̃T�C�Y */
	DWORD			dwCRC;				/* �i�[�t�@�C���̃`�F�b�N�T�� */
	UINT			uFlag;				/* �������� */
	UINT			uOSType;			/* ���ɍ쐬�Ɏg��ꂽ�n�r */
	WORD			wRatio;				/* ���k�� */
	WORD			wDate;				/* �i�[�t�@�C���̓��t(DOS �`��) */
	WORD 			wTime;				/* �i�[�t�@�C���̎���(�V) */
	char			szFileName[FNAME_MAX32 + 1];	/* ���ɖ� */
	char			dummy1[3];
	char			szAttribute[8];		/* �i�[�t�@�C���̑���(���ɌŗL) */
	char			szMode[8];			/* �i�[�t�@�C���̊i�[���[�h(�V) */
}	INDIVIDUALINFO, *LPINDIVIDUALINFO;

typedef struct {
	DWORD 			dwFileSize;		/* �i�[�t�@�C���̃T�C�Y */
	DWORD			dwWriteSize;	/* �������݃T�C�Y */
	char			szSourceFileName[FNAME_MAX32 + 1];	/* �i�[�t�@�C���� */
	char			dummy1[3];
	char			szDestFileName[FNAME_MAX32 + 1];
									/* �𓀐�܂��͈��k���p�X�� */
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