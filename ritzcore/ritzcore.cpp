// wavplay.cpp : DLL アプリケーション用にエクスポートされる関数を定義します。
//

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Windows ヘッダーから使用されていない部分を除外します。
// Windows ヘッダー ファイル:
#include <windows.h>
#include <mmsystem.h>
#include <math.h>
#include <time.h>

#include <mmintrin.h>   // MMX
#include <emmintrin.h>  // SSE2
#include <intrin.h>     // SSE3

#include "mame/emu.h"
#include "player.h"
#include "chiptrk.h"
#include "soundbuf.h"
#include "filters.h"

#include "mame/2151intf.h"
#include "mame/2413intf.h"
#include "mame/ay8910.h"
#include "mame/k051649.h"
#include "mame/nes_apu.h"



#define EXTERN_C extern "C"
/*
// これは、エクスポートされた変数の例です。
RITZAPI int npywinsnd=0;

// これは、エクスポートされた関数の例です。
RITZAPI int fnpywinsnd(void)
{
	return 42;
}
*/

class soundbox{
public:
	soundbox(device_t *device, int clock, int rate, int ntracks);
	virtual ~soundbox();
	device_t *const device;
	running_machine *const machine;
	const int sample_rate;
	void clock_forward(int clks);
	// UINT32 last_update_clk;
	virtual int read_mem(int adrs) = 0;
	virtual void write_mem(int adrs, int data) = 0;
	int get_ntracks(){ return ntracks; }
	chiptrack *get_track(int idx);
private:
	int ntracks;
	chiptrack **chip_tracks;
};


soundbox::soundbox(device_t *device, int clock, int rate, int ntracks)
:	device(device),
	sample_rate(rate),
	//last_update_clk(0),
	machine(new running_machine())
{
	this->ntracks = ntracks;
	device->set_machine(*machine);
	machine->m_cclk.set_inclock(rate, 1);
	chip_tracks = new chiptrack*[ntracks];
	for(int i=0; i<ntracks; i++){
		chip_tracks[i] = new chiptrack(rate);
	}
}
soundbox::~soundbox(){
	for(int i=0; i<ntracks; i++){
		delete chip_tracks[i];
	}
	delete[] chip_tracks;
	delete device;
	delete machine;
}

chiptrack *soundbox::get_track(int idx){
	if(idx<0 || ntracks<=idx)
		return NULL;
	return chip_tracks[idx];
}


void soundbox::clock_forward(int clks){
	// 時間をclks[sample clock]だけ時間を進める
	machine->sclock_forward(clks);
}


LARGE_INTEGER pf_freq;

void CALLBACK waveOutProc(
	HWAVEOUT hwo,
	UINT uMsg,
	DWORD_PTR dwInstance,
	DWORD_PTR dwParam1,
	DWORD_PTR dwParam2
){
	LPWAVEHDR lpwvhdr;
	SoundPlayer *player = (SoundPlayer*)dwInstance;
	switch(uMsg){
	case WOM_OPEN:
		break;
	case WOM_CLOSE:
		break;
	case WOM_DONE:	// 再生完了
		lpwvhdr = (LPWAVEHDR)dwParam1;
		SetEvent((HANDLE)lpwvhdr->dwUser);
	break;
	default:
		::Beep(880, 300);
		break;
	}
}


EXTERN_C HPLAYER RITZAPI wpPlayerOpen(){
	SoundPlayer *player;
	MMRESULT mmres;
	// player = new SoundPlayer;
	player = new SoundPlayer();
	//
	// 16bit 44.1kHz
	player->wfx.wFormatTag = WAVE_FORMAT_PCM;
	player->wfx.nChannels = 2;
	player->wfx.nSamplesPerSec = 44100;
	player->wfx.wBitsPerSample = 16;
	player->wfx.nBlockAlign = (player->wfx.nChannels * player->wfx.wBitsPerSample) >> 3;
	player->wfx.nAvgBytesPerSec = player->wfx.nSamplesPerSec * player->wfx.nBlockAlign;
	player->wfx.cbSize = 0;
	//
	mmres = waveOutOpen(
		&(player->h_waveout),
		WAVE_MAPPER,
		&(player->wfx),
		(DWORD_PTR)waveOutProc,
		(DWORD_PTR)player,
		CALLBACK_FUNCTION);
	if(mmres!=MMSYSERR_NOERROR){
		delete player;
		return NULL;
	}
	return (HPLAYER)player;
}
EXTERN_C int RITZAPI wpPlayerClose(HPLAYER hplayer){
	SoundPlayer *p = (SoundPlayer*)hplayer;
	waveOutClose(p->h_waveout);
	delete hplayer;
	return 0;
}

EXTERN_C int RITZAPI wpBufferEnqueue(HSNDBUF hbuf){
	SoundBuffer *buf = (SoundBuffer*)hbuf;
#ifdef _DEBUG
	if(buf->__MAGIC__ != 0xc0ffee){
		return -1;
	}
#endif
	// buf->wavehdr.dwBytesRecorded = buf->effective_samples * sizeof(UINT16) * 2;
	buf->wavehdr.dwBufferLength = buf->effective_samples * sizeof(RITZSAMP);
	ResetEvent((HANDLE)(buf->wavehdr.dwUser));
	waveOutWrite(
		buf->hwaveout,
		&buf->wavehdr,
		sizeof(WAVEHDR));
	return 0;
}


/********************************************************
 *
 * デバッグ関連
 *
#define DPRINTF ritz_dbg_printf
 ********************************************************/
CRITICAL_SECTION ritz_critical_section;

FILE *dbgfp = NULL;
void ritz_dbg_on(){
	::EnterCriticalSection(&ritz_critical_section);
	if(dbgfp==NULL){
		char buf[ 2048 ];
		DWORD pid = GetCurrentProcessId();
		strcpy(buf, "ritzdbg.txt");
		// sprintf_s(buf, sizeof(buf), "ritzdbg.txt", pid);
		fopen_s(&dbgfp, buf, "a+");
	}
	::LeaveCriticalSection(&ritz_critical_section);
}
void ritz_dbg_off(){
	::EnterCriticalSection(&ritz_critical_section);
	if(dbgfp!=NULL){
		fclose(dbgfp);
		dbgfp=NULL;
	}
	::LeaveCriticalSection(&ritz_critical_section);
}
void ritz_dbg_printf(const char *s_fmt, ...){
	va_list v;
	va_start( v, s_fmt );
	::EnterCriticalSection(&ritz_critical_section);
	if(dbgfp){
		DWORD msec = timeGetTime();
		fprintf(dbgfp, "%d:  ", msec & 0xfff);
		vfprintf(dbgfp, s_fmt, v);
		fflush(dbgfp);
	}
	::LeaveCriticalSection(&ritz_critical_section);
	va_end( v );
}


/********************************************************
 *
 * バッファ
 *
 ********************************************************/
EXTERN_C HSNDBUF RITZAPI wpBufferCreate(UINT32 size){
	SoundBuffer *buf = new SoundBuffer(size);
#ifdef _DEBUG
	buf->__MAGIC__ = 0xc0ffee;
#endif
	return (HSNDBUF)buf;
}

EXTERN_C int RITZAPI wpBufferDelete(HSNDBUF hbuf){
	SoundBuffer *buf = (SoundBuffer*)hbuf;
#ifdef _DEBUG
	if(buf->__MAGIC__ != 0xc0ffee){
		return -1;
	}
#endif
	delete buf;
	return 0;
}

EXTERN_C int RITZAPI wpBufferSetPlayable(HSNDBUF hbuf, HPLAYER hplayer){
	SoundBuffer *buf = (SoundBuffer*)hbuf;
	SoundPlayer *player = (SoundPlayer*)hplayer;
#ifdef _DEBUG
	if(buf->__MAGIC__ != 0xc0ffee){
		return -1;
	}
#endif
	return buf->setPlayable(player->h_waveout);
}

EXTERN_C RITZSAMP* RITZAPI wpBufferGetPointer(HSNDBUF hbuf){
	SoundBuffer *buf = (SoundBuffer*)hbuf;
#ifdef _DEBUG
	if(buf->__MAGIC__ != 0xc0ffee){
		return NULL;
	}
#endif
	return buf->buf;
}
EXTERN_C UINT32 RITZAPI wpBufferWait(HSNDBUF hbuf){
	// 再生終了まで待つ
	// 待った時間をマイクロ秒で返す
	LARGE_INTEGER t1, t2;
	SoundBuffer *buf = (SoundBuffer*)hbuf;
#ifdef _DEBUG
	if(buf->__MAGIC__ != 0xc0ffee){
		return NULL;
	}
#endif
	HANDLE hevent = (HANDLE)buf->wavehdr.dwUser;
	QueryPerformanceCounter(&t1);
	::WaitForSingleObject(hevent, INFINITE);
	QueryPerformanceCounter(&t2);
	buf->clear();
	double time =
		((t2.QuadPart - t1.QuadPart) * 1000.0 * 1000.0) /
		((double)pf_freq.QuadPart);
	return (UINT32)(time);
}

EXTERN_C int RITZAPI wpBufferSetData(HSNDBUF hbuf, RITZSAMP* data, UINT32 samples){
	SoundBuffer *buf = (SoundBuffer*)hbuf;
	samples = min(buf->samples, samples);
	if(data!=NULL){
		memcpy(buf->buf, data, sizeof(RITZSAMP)*samples);
	}
	buf->effective_samples = samples;
	return samples;
}

EXTERN_C UINT32 RITZAPI wpBufferCopy(HSNDBUF hbuf, void *outbuf, UINT32 outbuf_bytes){
	UINT32 my_bytes;
	SoundBuffer *sndbuf = (SoundBuffer*)hbuf;
	my_bytes = sndbuf->effective_samples * sizeof(RITZSAMP);
	if(outbuf_bytes<my_bytes)
		return my_bytes;
	memcpy(outbuf, sndbuf->buf, my_bytes);
	return my_bytes;
}

EXTERN_C void RITZAPI wpBufferClear(HSNDBUF hbuf){
	SoundBuffer *buf = (SoundBuffer*)hbuf;
	buf->clear();
}

/****************************************************************
 *
 * フィルタ
 *
 ****************************************************************/
EXTERN_C HFILTER RITZAPI filter_create(int type){
	FilterBase *filter = NULL;
	switch(type){
	default:
		filter = new FIRFilter();
		break;
	}
	return (HFILTER)filter;
}
EXTERN_C void RITZAPI filter_delete(HFILTER hfilter){
	FilterBase *filter = (FilterBase*)hfilter;
	delete filter;
}

EXTERN_C void RITZAPI filter_apply(HFILTER hfilter, HSNDBUF hsndbuf){
	FilterBase *filter = (FilterBase*)hfilter;
	SoundBuffer *sndbuf = (SoundBuffer*)hsndbuf;
	filter->apply(sndbuf);
}



EXTERN_C int RITZAPI filter_set_fir_coeffs(HFILTER hfilter, int ntaps, float*args){
	FIRFilter *filter = (FIRFilter*)hfilter;
	// TODO: フィルタ種別チェック
	filter->setFirCoeffs(ntaps, args);
	return 0;
}


/****************************************************************
 *
 * モジュール共通
 *
 ****************************************************************/

// AY8910
void* AY8910_init(int clock, int rate);
// SCC
void* scc_init(int clock, int rate);
// Famicom APU
void* nes_init(int clock, int rate);
// OPM
void* ritz_ym2151_init(int clock, int rate);
// OPLL
void* ritz_ym2413_init(int clock, int rate);

typedef  void*(*MODCREATOR)(int clock, int rate);
struct module_creator_info{
	const char* name;
	MODCREATOR mcfunc;
} module_creators[] = {
	{"2312P001", scc_init},
	{"ay8910", AY8910_init},
	{"RP2A03", nes_init},
	{"ym2151", ritz_ym2151_init},
	{"ym2413", ritz_ym2413_init},
	{NULL, NULL},
};

EXTERN_C void* RITZAPI mod_create(const char* name, int clock, int rate)
	/* モジュールの作成
	 * name: モジュール名
	 */
{
	struct module_creator_info* ptr;
	void* module = NULL;
	for(ptr=module_creators; ptr->name!=NULL; ptr++){
		if(strcmp(ptr->name, name)==0){
			//found!
			module = ptr->mcfunc(clock, rate);
			break;
		}}
	return module;
}

EXTERN_C void  RITZAPI mod_reset(void* handle){
	soundbox *box = (soundbox*)handle;
	box->device->reset();
}

EXTERN_C void  RITZAPI mod_shutdown(void* handle){
	soundbox *box = (soundbox*)handle;
	delete box;
}


EXTERN_C float RITZAPI mod_get_gain(void *handle){
	soundbox *box = (soundbox*)handle;
	return box->device->machine().stream()->get_gain();
}

EXTERN_C void  RITZAPI mod_clock_forward(void *handle, int clks){
	soundbox *box = (soundbox*)handle;
	box->clock_forward(clks);
}

EXTERN_C void RITZAPI mod_set_gain(void *handle, float gain){
	soundbox *box = (soundbox*)handle;
	return box->device->machine().stream()->set_gain(gain);
}


EXTERN_C void  RITZAPI mod_update(void *handle){
	soundbox *box = (soundbox*)handle;
	sound_stream *stream;
	int length;
	// 現在時刻までのデータを生成
	stream = box->machine->stream();
	stream->update();
	// 出力サンプル数を取得
	length = box->machine->sclock();
	// リサンプリング
	//stream->generate_resampled_data(box->sample_rate, buffers, length);
	for(int ch=0; ch<box->get_ntracks(); ch++){
		stream->generate_resampled_data(ch, box->sample_rate, box->get_track(ch), length);
	}
	stream->update_with_accounting();
	//box->last_update_clk = now;
	box->machine->sclock_reset();
}


EXTERN_C void  RITZAPI mod_mixto(void *handle, HSNDBUF hsndbuf, float gain){
	// chipのトラックから再生バッファへミックスする
	soundbox *box = (soundbox*)handle;
	SoundBuffer *sndbuf = (SoundBuffer*)hsndbuf;
	int ntracks;
	_mm_prefetch((const CHAR*)sndbuf->buf, _MM_HINT_T0);
	ntracks = box->get_ntracks();

	// gain limiter
	if(gain<-3.0f){
		gain = -3.0f;
	}else if(3.0f<gain){
		gain = 3.0f;
	}

	if(ntracks==1){ // MONO
		register RITZSAMP *ptr = sndbuf->buf;
		chiptrack *trk = box->get_track(0);
		RITZSAMP *src = trk->get_pointer();
		_mm_prefetch((const CHAR*)src, _MM_HINT_T2);
		float gain_l = trk->get_gain_l() * gain;
		float gain_r = trk->get_gain_r() * gain;
		size_t width = trk->get_effective_samples();
		for(size_t x=0; x<width; x++){
			RITZSAMP data = *(src++);
			*(ptr++) += (RITZSAMP)(data * gain_l);
			*(ptr++) += (RITZSAMP)(data * gain_r);
		}
		sndbuf->effective_samples = trk->get_effective_samples() * 2;//ステレオだから*2
	} else { // STEREO
		for(int t=0; t<box->get_ntracks(); t++){
			register RITZSAMP *ptr = sndbuf->buf;
			chiptrack *trk = box->get_track(t);
			RITZSAMP *src = trk->get_pointer();
			_mm_prefetch((const CHAR*)src, _MM_HINT_T2);
			float gain_l = trk->get_gain_l() * gain;
			float gain_r = trk->get_gain_r() * gain;
			size_t width = trk->get_effective_samples();
			for(size_t x=0; x<width; x++){
				RITZSAMP data = *(src++);
				RITZSAMP data_l, data_r;
				data_l = data_r = data;
				if(t==0) data_r = 0;
				if(t==1) data_l = 0;
				*(ptr++) += (RITZSAMP)(data_l * gain_l);
				*(ptr++) += (RITZSAMP)(data_r * gain_r);
			}
			sndbuf->effective_samples = trk->get_effective_samples() * 2;//ステレオだから*2
		}
	}
}

EXTERN_C int   RITZAPI mod_read(void *handle, int adrs){
	soundbox *box = (soundbox*)handle;
	return box->read_mem(adrs);
}

EXTERN_C void  RITZAPI mod_write(void *handle, int adrs, int data){
	soundbox *box = (soundbox*)handle;
	box->write_mem(adrs, data);
}



/****************************************************************
 *
 * AY8910
 *
 ****************************************************************/
static const char* AY8910_MAGIC = "AY8910";
static const ay8910_interface ay8910_conf = {
	AY8910_LEGACY_OUTPUT | AY8910_SINGLE_OUTPUT,
	//AY8910_DEFAULT_LOADS,
	//{1000, 1000, 1000},
	{100, 100, 100},
};
class box_AY8910: public soundbox{
public:
	box_AY8910(device_t *device, int clock, int rate): soundbox(device, clock, rate, 1){}
	virtual int read_mem(int adrs);
	virtual void write_mem(int adrs, int data);
};

void* AY8910_init(int clock, int rate){
	box_AY8910 *box;
	device_t *dev = new ay8910_device(NULL, NULL, NULL, clock);
	device_t::static_set_static_config(*dev, &ay8910_conf);
	box = new box_AY8910(dev, clock, rate);
	box->device->start();
	return (void*)box;
}
int box_AY8910::read_mem(int adrs){
	ay8910_device *device = (ay8910_device *)this->device;
	return device->data_r(0);
}
void box_AY8910::write_mem(int adrs, int data){
	ay8910_device *device = (ay8910_device *)this->device;
	device->address_data_w(0, adrs);
	device->address_data_w(1, data);
}

/****************************************************************
 *
 * SCC
 *
 ****************************************************************/
class box_scc: public soundbox{
public:
	box_scc(device_t *device, int clock, int rate): soundbox(device, clock, rate, 1){}
	virtual int read_mem(int adrs);
	virtual void write_mem(int adrs, int data);
};
void* scc_init(int clock, int rate){
	box_scc *box;
	device_t *dev = new k051649_device(NULL, NULL, NULL, clock);
	box = new box_scc(dev, clock, rate);
	box->device->start();
	return (void*)box;
}

int box_scc::read_mem(int adrs){
	return 0xff;
}

/*
00-1FH　チャンネルＡ用波形データ
20-9FH  同様に、３２バイトずつチャンネルＢ～Ｅ用波形データ

A0-A1H  チャンネルＡ用分周比
A2-A9H  同様に、２バイトずつチャンネルＢ～Ｅ用分周比

AAH       チャンネルＡ用音量
AB-AEH  同様に、１バイトずつチャンネルＢ～Ｅ用音量

AFH       チャンネルイネーブル。

C0-DFH  モード設定レジスタ
*/
void box_scc::write_mem(int addr_, int data){
	k051649_device *device = (k051649_device *)this->device;
	int adrs = addr_ & 0xff;
	if(adrs<0xa0){ // waveform
		device->k052539_waveform_w(adrs, data);
	}else if(adrs<0xaa){ // division ratio
		device->k051649_frequency_w(adrs&0x0f, data);
	}else if(adrs<0xaf){
		device->k051649_volume_w(adrs-0xaa, data);
	}else if(adrs==0xaf){
		device->k051649_keyonoff_w(0, data);
	}else{
		adrs++;
	}
}

/****************************************************************
 *
 * Famicom
 *
 ****************************************************************
矩形波
レジスタに設定する値 = CPUのクロック周波数 / (再生したい周波数 * 32) - 1
三角波
矩形波より1オクターブ低い音が出る
 */
class box_nes: public soundbox{
public:
	box_nes(device_t *device, int clock, int rate): soundbox(device, clock, rate, 1){}
	virtual int read_mem(int adrs);
	virtual void write_mem(int adrs, int data);
};
void* nes_init(int clock, int rate){
	box_nes *box;
	device_t *device = new nesapu_device(NULL, NULL, NULL, clock);
	box = new box_nes(device, clock, rate);
	// メモリ確保
	/*
	 *再生開始アドレスとして指定できるのは、$C000～$FFC0まで。$0000～$BFFFは指定できない。
	 *しかし、再生の際にデータを$FFFFまで読んだ後は、$8000から読み続ける。
	 */
	address_space *mem = new address_space(0x8000, 0x8000);
	box->machine->set_memory(mem);
	box->device->start();
	return (void*)box;
}

int box_nes::read_mem(int addr_){
	nesapu_device* device = (nesapu_device*)this->device;
	int adrs = addr_ & 0xffff;
	int bank = (addr_>>8) & 0xff;
	if(bank==0x40){ // APUレジスタ
		return device->read(adrs & 0x1f);
	}
	return 0xff;
}

void box_nes::write_mem(int addr_, int v){
	int adrs = addr_ & 0xffff;
	int bank = (addr_>>8) & 0xff;
	if(bank==0x40){ // APUレジスタ
		nesapu_device* device = (nesapu_device*)this->device;
		device->write(adrs & 0x1f, v);
	} else if(0!=(bank & 0x80)){
		// DPCM用メモリ(0x8000-0xffff)
		this->machine->m_memory->write_byte(adrs, v);
	}
}


/****************************************************************
 *
 * YM2151 OPM
 *
 ****************************************************************/
class box_ym2151: public soundbox{
public:
	box_ym2151(device_t *device, int clock, int rate): soundbox(device, clock, rate, 2){}
	virtual int read_mem(int adrs);
	virtual void write_mem(int adrs, int data);
};
void* ritz_ym2151_init(int clock, int rate){
	box_ym2151 *box;
	device_t *device = new ym2151_device(NULL, NULL, NULL, clock);
	box = new box_ym2151(device, clock, rate);
	box->device->start();
	return (void*)box;
}

int box_ym2151::read_mem(int adrs_){
	int adrs = adrs_ & 0xff;
	ym2151_device* device = (ym2151_device*)this->device;
	return device->read(adrs);
}
void box_ym2151::write_mem(int adrs_, int v){
	int adrs = adrs_ & 0xff;
	ym2151_device* device = (ym2151_device*)this->device;
	device->write(adrs, v);
}

/****************************************************************
 *
 * YM2413 OPLL
 *
 ****************************************************************/
class box_opll: public soundbox{
public:
	box_opll(device_t *device, int clock, int rate): soundbox(device, clock, rate, 2){}
	virtual int read_mem(int adrs);
	virtual void write_mem(int adrs, int data);
};
void* ritz_ym2413_init(int clock, int rate){
	box_opll *box;
	device_t *device = new ym2413_device(NULL, NULL, NULL, clock);
	box = new box_opll(device, clock, rate);
	box->device->start();
	return (void*)box;
}

int box_opll::read_mem(int adrs_){
	return 0xff;
}
void box_opll::write_mem(int adrs_, int v){
	int adrs = adrs_ & 0xff;
	ym2413_device* device = (ym2413_device*)this->device;
	device->write(adrs, v);
}
