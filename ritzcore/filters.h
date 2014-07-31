#ifndef FILTERS_H_INCLUDED__
#define FILTERS_H_INCLUDED__


class SoundBuffer;

class FilterBase{
protected:
	const int magic;
	const int filter_type;
public:
	FilterBase(int type);
	virtual ~FilterBase();
	virtual void apply(SoundBuffer*) = 0;
};


class FIRFilter: public FilterBase{
public:
	FIRFilter();
	virtual ~FIRFilter();
	void setFirCoeffs(int ntaps, float *coeffs);
	virtual void apply(SoundBuffer*);
private:
	int nTaps;
	float *coeffs;
	float *inpbuf;
	size_t inpbuf_len;
	size_t inpbuf_pos;

};




#endif /* FILTERS_H_INCLUDED__ */
