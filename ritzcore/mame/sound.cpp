
#include "emu.h"
#include "device.h"
#include "sound.h"
#include "../chiptrk.h"

sound_stream::sound_stream(device_t &device, int inputs, int outputs, int sample_rate, void *param, stream_update_func callback)
	: m_device(device),
		//m_next(NULL),
		m_sample_rate(sample_rate),
		m_new_sample_rate(0),
		//m_attoseconds_per_sample(0),
		//m_max_samples_per_update(0),
		//m_input(inputs),
		//m_input_array(inputs),
		//m_resample_bufalloc(0),
		m_output(outputs), // 出力チャンネル数が入る
		//m_output_array(outputs),
		//m_output_bufalloc(0),
		m_output_sampindex(0),
		//m_output_update_sampindex(0),
		//m_output_base_sampindex(0),
		m_callback(callback),
		m_param(param),
		m_last_update_clock(0)
{
	size_t seclen = sample_rate * sizeof(stream_sample_t);
	raw_buffers = new stream_sample_t*[m_output];
	//DBG_ON();
	for(int i=0; i<m_output; i++){
		raw_buffers[i] = (stream_sample_t*)RitzAlloc(seclen);
		DPRINTF("raw buffer: %d\n", seclen);
	}
	m_gain = 1.0f;
	if (m_callback == &sound_stream::device_stream_update_stub)
		m_param = &device;
	device.machine().m_cclk.set_outclock(sample_rate, 1);
	DBG_OFF();
}


sound_stream::~sound_stream(){
	for(int i=0; i<m_output; i++)
		RitzFree(raw_buffers[i]);
	delete[] raw_buffers;
}

void sound_stream::set_sample_rate(int new_rate)
{
	// we will update this on the next global update
	///TODO: implement.
	///if (new_rate != sample_rate())
	///	m_new_sample_rate = new_rate;
}


//-------------------------------------------------
//  update_with_accounting - do a regular update,
//  but also do periodic accounting
//-------------------------------------------------
void sound_stream::update_with_accounting()
{
	// do the normal update
	update();
	m_output_sampindex = 0;
	m_last_update_clock = 0;
}

void sound_stream::update(){
	int sclks, now;
	UINT32 update_sampindex;
	// 前回からの経過時間[sample clock数]を求める
	now = m_device.machine().sclock();
	sclks = now - m_last_update_clock;
	//if(sclks == 0)
	//	return; // already updated
	//DBG_ON();
	//DPRINTF("clk: sclks:%d %d %d\n", sclks, now, m_last_update_clock);
	//if(sclks<0)
	//	sclks += MACHINE_CLOCK_MASK + 1;
	// 生成する内部クロック数
	update_sampindex = m_device.machine().m_cclk.tick(sclks);
	if(update_sampindex==0)
		return;
	generate_samples(update_sampindex);
	// remember this info for next time
	DPRINTF("sclks0:%d %d\n", update_sampindex, m_output_sampindex);
	m_output_sampindex += update_sampindex;
	DPRINTF("sclkss:%d %d %d\n",
		sclks, update_sampindex, m_output_sampindex);
	m_last_update_clock = now;
	DBG_OFF();
}

#if 0
void sound_stream::update(){
	unsigned int now = m_device.machine().time16();
	INT32 update_sampindex;
	int dt;
	// 前回からの経過時間[msec]をdtに求める
	if(now < m_last_update_time){
		// timer overflow
		dt = 0x10000 + now - m_last_update_time;
	} else {
		dt = now - m_last_update_time;
	}
	if(dt==0) // already updated.
		return;
	// dt[msec]はサンプリング周波数何発分?
	update_sampindex = m_sample_rate * dt / 1000;
	// あまりの部分を処理
	clock_remainder += m_sample_rate * dt - update_sampindex * 1000;
	while(1000<=clock_remainder){
		update_sampindex++;
		clock_remainder -= 1000;
	}
	//
	generate_samples(update_sampindex - m_output_sampindex);
	// remember this info for next time
	m_output_sampindex = update_sampindex;
	m_last_update_time = now;
}
#endif

void sound_stream::generate_samples(int samples){
	//チップ依存のサンプリングレートを使用している
	stream_sample_t **m_output_array;
	// if we're already there, skip it
	if (samples <= 0)
		return;
	m_output_array = new stream_sample_t*[m_output];
	for(int i=0; i<m_output; i++)
		m_output_array[i] = raw_buffers[i] + m_output_sampindex;
	//DBG_ON();
	//DPRINTF("generate_samples: samples: %d, offset: %d\n", samples, m_output_sampindex);
	//DPRINTF("buflen %d\n", sample_rate() * sizeof(stream_sample_t));
	(*m_callback)(&m_device, this, m_param,
		NULL /*m_input_array*/,
		m_output_array, samples);
	delete[] m_output_array;
	//DPRINTF("generate_samples<<<\n");
	//DBG_OFF();
}

//-------------------------------------------------
//  generate_resampled_data - generate the
//  resample buffer for a given input
//-------------------------------------------------
void sound_stream::generate_resampled_data(int itrack, int rate, chiptrack *outtrk, UINT32 numsamples)
	/*
	rate: 出力サンプリングレート
	out: 出力先
	numsamples: 出力するサンプル数
	*/
{
	stream_sample_t *rptr = raw_buffers[itrack];
	int att = outtrk->get_attenuator();
#define GAINED(x) ((int)(x)>>att)
	if(rate<m_sample_rate){
		// データを間引く
		int x = 0;
		signed __int16 *out = outtrk->get_pointer();

		outtrk->set_effective_samples(numsamples);
		while(0<numsamples){
			x += m_sample_rate;  //m_sample_rate: chip内部のサンプリングレート
			int a = x / rate;	 //rate: 出力サンプリングレート
			*out++ = GAINED(*rptr);
			x -= rate * a;
			numsamples--;
			rptr += a;
		}
	} else {
		//TODO: 実装
		outtrk->set_effective_samples(0);
	}
#undef GAINED

#if 0
	int c1, c2;
	//TODO: 3channel以上への対応? えー
	stream_sample_t *rbuf, *lbuf;
	if(m_output==1){ //source is mono
		lbuf = rbuf = raw_buffers[0];
	} else {
		lbuf = raw_buffers[0];
		rbuf = raw_buffers[1];
	}

	c1 = c2 = 0;
	if(m_sample_rate==rate){
		size_t bytes;
		bytes = sizeof(stream_sample_t)*numsamples;
		memcpy(out[0], lbuf, bytes);
		memcpy(out[1], rbuf, bytes);
	} else if(rate<m_sample_rate){
		// データを間引く
		int x = 0;
		stream_sample_t *l = out[0];
		stream_sample_t *r = out[1];
		//DBG_ON();
		//DPRINTF("W: 0x%x  %d\n", l, numsamples);
		while(0<numsamples){
			x += m_sample_rate;  //m_sample_rate: chip内部のサンプリングレート
			int a = x / rate;	 //rate: 出力サンプリングレート
			*l++ = GAINED(*lbuf);
			*r++ = GAINED(*rbuf);
			x -= rate * a;
			numsamples--;

			lbuf += a;
			rbuf += a;
		}
		//DBG_OFF();
	} else {
		// データを補完する not tested.
		int rm = 0;
		int x = rate;
		stream_sample_t *l = out[0];
		stream_sample_t *r = out[1];
		while(0<numsamples){
			x -= m_sample_rate;
			if(x<=0){
				*l++ = GAINED(*lbuf++);
				*r++ = GAINED(*rbuf++);
				x += rate;
			} else {
				*l++ = *lbuf;
				*r++ = *rbuf;
			}
			numsamples--;
		}
	}
#endif
}

//-------------------------------------------------
//  device_stream_update_stub - stub callback for
//  passing through to modern devices
//-------------------------------------------------
STREAM_UPDATE( sound_stream::device_stream_update_stub )
{
	//device_sound_interface *sound = reinterpret_cast<device_sound_interface *>(param);
	device_t *sound = reinterpret_cast<device_t *>(param);
	sound->sound_stream_update(*stream, inputs, outputs, samples);
}
