#ifndef CHIPTRK_H_INCLUDED__
#define CHIPTRK_H_INCLUDED__

class chiptrack{
public:
	chiptrack(__int32 length);
	virtual ~chiptrack();
	void setPan(float alpha);
	signed __int16* get_pointer(){ return sndbuf; }
	int get_attenuator(){ return attenuator; }
	void set_effective_samples(size_t x){ effective_samples=x; }
	size_t get_effective_samples(){ return effective_samples; }
	float get_gain_r(){ return gain_r; }
	float get_gain_l(){ return gain_l; }
private:
	signed __int16* sndbuf;
	size_t effective_samples;
	int attenuator;
	float gain_l, gain_r;
};


#endif /* CHIPTRK_INCLUDED__ */
