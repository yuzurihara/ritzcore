
#ifndef CCONV_H_INCLUDED__
#define CCONV_H_INCLUDED__


class cconv{
public:
	cconv();
	virtual ~cconv();
	void set_inclock(int icn, int icd);
	void set_outclock(int ocn, int ocd);
	int tick(int clks);

private:
	int tick1(int clks);
	int tick2(int clks);

	void update();
	// in-clock's numerator & denominator
	__int64 icn, icd;
	// out-clock's numerator & denominator
	__int64 ocn, ocd;
	//
	int m_a, m_b;
	int m_d;	// denominator
	int m_rem;	// clock remainder
	__int64 m_rem2;	// clock remainder

	int _c1;
};

#endif /* CCONV_H_INCLUDED__ */