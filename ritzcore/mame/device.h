

#ifndef DEVICE_H_INCLUDED__
#define DEVICE_H_INCLUDED__

// クロックは24bit
#define MACHINE_CLOCK_MASK 0xffffff

typedef enum device_type_{
	// PSG type
	AY8910 = 1,
	AY8914,
	AY8930,
	// Konami SCC
	K051649,
	// famicom
	NES_APU,
	//YM2151,
	//YM2413,
	DEVICE_TYPE_YM2413,
} device_type;




class sound_stream;
class device_t;

typedef void* machine_config;


/*** running_machine ***/

class running_machine{
public:
	running_machine();
	~running_machine();
	UINT32 sclock(){ return m_sclk; }
	void sclock_forward(UINT32 clks);
	void sclock_reset(){ m_sclk=0; }

	sound_stream *stream_alloc(device_t &device,
		int inputs, int outputs, int sample_rate,
		void *param=NULL, sound_stream::stream_update_func callback= &sound_stream::device_stream_update_stub);
	sound_stream* stream(){ return m_stream; }
	void set_memory(address_space *);

	cconv m_cclk;	// クロックコンバータ
	address_space *m_memory;  // メモリ

private:
	UINT32 m_sclk; // サンプリング周波数で動作するカウンタ
	sound_stream *m_stream;
};

/*** device_t ***/

class device_t{
public:
	virtual ~device_t(){};

	device_type type(){ return m_type; }
	int clock(){ return m_clock; }
	running_machine &machine() const { return *m_machine; }
	void set_machine(running_machine &machine){ m_machine = &machine; }
	sound_stream *stream_alloc(int inputs, int outputs, int sample_rate);
	//
	void reset();
	void start();
	// sound stream update overrides
	virtual void sound_stream_update(sound_stream &stream, stream_sample_t **inputs, stream_sample_t **outputs, int samples){}
	// interrupt handle
	// 割り込み要求が上がっているか?
	bool has_irq(int type);
	// 割り込み要求を取り下げる
	bool clear_irq(int type);
	const void *static_config() const { return m_static_config; }
	static void static_set_static_config(device_t &device, const void *config) { device.m_static_config = config; }

	//{ダミー関数
	template<typename _ItemType>
	void save_item(_ItemType &value, const char *valname, int index = 0){}
	//ダミー関数}

protected:
	device_t(device_type type, int clock);
	device_t(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock, const char *shortname, const char *source);

	virtual void device_reset(){}
	virtual void device_start(){}
	// IRQ
	int		m_input[MAX_INPUT_LINES];
	void set_input_line(int line, int state);
	const void *            m_static_config;        // static device configuration

protected:
	running_machine *       m_machine;
	device_type m_type;
	int m_clock;		// デバイスの動作クロック

};


/* from disound.h */
class device_sound_interface{
public:
	device_sound_interface(const machine_config &mconfig, device_t &device){};
};

#endif DEVICE_H_INCLUDED__