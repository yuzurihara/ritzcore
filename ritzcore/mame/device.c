
#include "emu.h"
#include "device.h"

/****************************************************************
 *
 * running_machine
 *
 ****************************************************************/
// クロックは24bit
running_machine::running_machine()
:	m_sclk(0),
	m_stream(NULL),
	m_memory(NULL)
{
}
running_machine::~running_machine(){
	if(m_stream)
		delete m_stream;
	if(m_memory)
		delete m_memory;
}

void running_machine::sclock_forward(UINT32 clks){
	m_sclk = (m_sclk + clks) & MACHINE_CLOCK_MASK;
}
/*
UINT32 running_machine::elapse_clocks(UINT32 t0){
	INT32 diff = m_clks - t0;
	if(diff<0)
		diff += (MACHINE_CLOCK_MASK+1);
	return (UINT32)diff;
}
*/
void running_machine::set_memory(address_space *m){
	if(m_memory)
		delete m_memory;
	m_memory = m;
}

sound_stream *running_machine::stream_alloc(
	device_t& device,
	int inputs, int outputs, int sample_rate,
	void *param, sound_stream::stream_update_func callback)
{
	// TODO: 多重ストリームの禁止check
	sound_stream* stream;
	if (callback != NULL){
		stream = new sound_stream(device, inputs, outputs, sample_rate, param, callback);
	} else {
		stream = new sound_stream(device, inputs, outputs, sample_rate);
	}
	m_stream = stream;
	return stream;
}


/****************************************************************
 *
 * device_t
 *
 ****************************************************************/
device_t::device_t(device_type type, int clock)
:	m_static_config(NULL)
{
	m_type = type;
	m_clock = clock;
	for(int i=0; i<MAX_INPUT_LINES; i++)
		m_input[i] = CLEAR_LINE;
}

device_t::device_t(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source){
	m_type = type;
	m_clock = clock;
	for(int i=0; i<MAX_INPUT_LINES; i++)
		m_input[i] = CLEAR_LINE;
}



sound_stream *device_t::stream_alloc(int inputs, int outputs, int sample_rate)
{
	return machine().stream_alloc(*this, inputs, outputs, sample_rate);
}

void device_t::reset(){
	// reset the device
	device_reset();
}

void device_t::start(){
	// start the device
	device_start();
}


void device_t::set_input_line(int line, int state){
	m_input[line] = state;
}

// 割り込み要求が上がっているか?
bool device_t::has_irq(int line){
	bool sig;
	switch(m_input[line]){
	case ASSERT_LINE:
		// 自動で割り込みシグナルをリセット TODO: 要調査
		m_input[line] = CLEAR_LINE;
		sig = true;
		break;
	case HOLD_LINE:	
		// 割り込みシグナルをリセットしない TODO: 要調査
		sig = true;
		break;
	default:
		sig = false;
		break;
	}
	return sig;
}
// 割り込み要求を取り下げる
bool device_t::clear_irq(int line){
	int oldval = m_input[line];
	m_input[line] = CLEAR_LINE;
	return oldval;
}


