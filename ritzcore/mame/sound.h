
#ifndef SOUND_H_INCLUDED__
#define SOUND_H_INCLUDED__

#define STREAM_UPDATE(name) void name(device_t *device, sound_stream *stream, void *param, stream_sample_t **inputs, stream_sample_t **outputs, int samples)

class sound_stream{
	friend class running_machine;
public:
	typedef void (*stream_update_func)(device_t *device, sound_stream *stream, void *param, stream_sample_t **inputs, stream_sample_t **outputs, int samples);
	sound_stream(device_t &device,
		int inputs, int outputs, int sample_rate,
		void *param=NULL, stream_update_func callback = &sound_stream::device_stream_update_stub);

	virtual ~sound_stream();

	int sample_rate() const { return m_sample_rate; } // { return (m_new_sample_rate != 0) ? m_new_sample_rate : m_sample_rate; }

	void update();
	void set_sample_rate(int new_rate);

	void update_with_accounting();
	stream_sample_t *generate_resampled_data(UINT32 numsamples);
	void generate_resampled_data(int itrack, int rate, class chiptrack *out, UINT32 numsamples);
	float get_gain(){ return m_gain; }
	void set_gain(float gain){ m_gain=gain; }

private:
	INT32               m_output_sampindex;     // current position within each output buffer
	//INT32               m_output_update_sampindex;// position at time of last global update

	// unsigned int m_last_update_time;
	int m_last_update_clock;

	void generate_samples(int samples);

	// mebmers
	float m_gain;
	int m_output;
	device_t &          m_device;
	int m_sample_rate;
	int m_new_sample_rate;
	stream_update_func m_callback;
	void *              m_param;

	// サウンドバッファ(1秒分) サンプリングレートはchip依存
	stream_sample_t **raw_buffers;
	static STREAM_UPDATE( device_stream_update_stub );

};

class running_machine;

class sound_manager{
	friend class sound_stream;
public:
	sound_manager(running_machine &machine);
	~sound_manager();

	running_machine &machine() const { return m_machine; }
	sound_stream *stream_alloc(device_t &device, int inputs, int outputs, int sample_rate, void *param = NULL, sound_stream::stream_update_func callback = NULL);

private:
	running_machine &   m_machine;              // reference to our machine
};



#endif /* SOUND_H_INCLUDED__ */
