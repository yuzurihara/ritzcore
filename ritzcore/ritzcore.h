// �ȉ��� ifdef �u���b�N�� DLL ����̃G�N�X�|�[�g��e�Ղɂ���}�N�����쐬���邽�߂� 
// ��ʓI�ȕ��@�ł��B���� DLL ���̂��ׂẴt�@�C���́A�R�}���h ���C���Œ�`���ꂽ PYWINSND_EXPORTS
// �V���{�����g�p���ăR���p�C������܂��B���̃V���{���́A���� DLL ���g�p����v���W�F�N�g�ł͒�`�ł��܂���B
// �\�[�X�t�@�C�������̃t�@�C�����܂�ł��鑼�̃v���W�F�N�g�́A 
// RITZAPI �֐��� DLL ����C���|�[�g���ꂽ�ƌ��Ȃ��̂ɑ΂��A���� DLL �́A���̃}�N���Œ�`���ꂽ
// �V���{�����G�N�X�|�[�g���ꂽ�ƌ��Ȃ��܂��B
#ifndef RITZCORE_H_INCLUDED__
#define RITZCORE_H_INCLUDED__

#include <windows.h>
#include "mame/emu.h"


/*
#ifdef PYWINSND_EXPORTS
#define RITZAPI __declspec(dllexport)
#else
#define RITZAPI __declspec(dllimport)
#endif


// ���̃N���X�� wavplay.dll ����G�N�X�|�[�g����܂����B
class RITZAPI Cpywinsnd {
public:
	Cpywinsnd(void);
	// TODO: ���\�b�h�������ɒǉ����Ă��������B
};

extern RITZAPI int npywinsnd;

RITZAPI int fnpywinsnd(void);
*/
#define RITZAPI __stdcall

#define SND_CHANNELS 2
 
extern const HANDLE gProcessHeap;

#define RitzAlloc(x) HeapAlloc(gProcessHeap, HEAP_GENERATE_EXCEPTIONS|HEAP_ZERO_MEMORY, x)
#define RitzFree(x) HeapFree(gProcessHeap, 0, x)
//#define AMALLOC(x) HeapAlloc(gProcessHeap, HEAP_GENERATE_EXCEPTIONS|HEAP_ZERO_MEMORY, x)
//#define AFREE(x) HeapFree(gProcessHeap, 0, x)
#define AMALLOC(x) _aligned_malloc(x, 16)
#define AFREE(x) _aligned_free(x)


/*
typedef class SoundPlayer* HPLAYER;
typedef class SoundBuffer* HSNDBUF;
typedef class SoundBuffer* HSNDBUF;
*/
typedef struct HPLAYER_{
	int *dmy;
} *HPLAYER;
typedef struct HSNDBUF_{
	int *dmy;
} *HSNDBUF;
typedef struct HFILTER_{
	int *dmy;
} *HFILTER;

typedef signed __int16 RITZSAMP;



// �o�b�t�@�֘A
EXTERN_C HSNDBUF RITZAPI wpBufferCreate(UINT32 size);
EXTERN_C int RITZAPI wpBufferDelete(HSNDBUF);
EXTERN_C int RITZAPI wpBufferSetPlayable(HSNDBUF hbuf, HPLAYER hplayer);
EXTERN_C RITZSAMP* RITZAPI wpBufferGetPointer(HSNDBUF hbuf);
EXTERN_C int RITZAPI wpBufferSetData(HSNDBUF hbuf, RITZSAMP*, UINT32 samples);
EXTERN_C UINT32 RITZAPI wpBufferCopy(HSNDBUF hbuf, void *outbuf, UINT32 outbuf_bytes);
EXTERN_C void RITZAPI wpBufferClear(HSNDBUF hbuf);
EXTERN_C int RITZAPI wpBufferEnqueue(HSNDBUF);
/*** Player
 *
 *
 *
 */
// PLAYER
EXTERN_C HPLAYER RITZAPI wpPlayerOpen();
EXTERN_C int RITZAPI wpPlayerClose(HPLAYER);
EXTERN_C void RITZAPI wpPlayerSetFirCoefficients(HPLAYER hplayer, int ntaps, float* args);

// FILTER
EXTERN_C HFILTER RITZAPI filter_create(int);
EXTERN_C void    RITZAPI filter_delete(HFILTER);
EXTERN_C int     RITZAPI filter_set_fir_coeffs(HFILTER, int ntaps, float*args);
EXTERN_C void    RITZAPI filter_apply(HFILTER, HSNDBUF);

// Module common
EXTERN_C void  RITZAPI mod_mixto(void *handle, HSNDBUF, float gain);
EXTERN_C void  RITZAPI mod_update(void *handle);
EXTERN_C float RITZAPI mod_get_gain(void *handle);
EXTERN_C void  RITZAPI mod_set_gain(void *handle, float gain);
EXTERN_C void  RITZAPI mod_clock_forward(void *handle, int clks);

EXTERN_C void* RITZAPI mod_create(const char* name, int clock, int rate);
EXTERN_C void  RITZAPI mod_shutdown(void* handle);
EXTERN_C void  RITZAPI mod_reset(void* handle);
EXTERN_C int   RITZAPI mod_read(void *handle, int adrs);
EXTERN_C void  RITZAPI mod_write(void *handle, int adrs, int data);


extern FILE *dbgfp;
#define DPRINTF ritz_dbg_printf

#define DBG_ON ritz_dbg_on
#define DBG_OFF ritz_dbg_off

void ritz_dbg_printf(const char *s_fmt, ...);
void ritz_dbg_on();
void ritz_dbg_off();


extern CRITICAL_SECTION ritz_critical_section;

#endif /* RITZCORE_H_INCLUDED__ */
// [EOF]
