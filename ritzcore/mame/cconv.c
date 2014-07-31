
#include "emu.h"
#include "cconv.h"

cconv::cconv()
{
	icn = icd = 1;
	set_outclock(icn, icd);
	_c1 = 0;
}
cconv::~cconv(){
}

void cconv::set_inclock(int icn, int icd){
	this->icn = icn;
	this->icd = icd;
	update();
}

void cconv::set_outclock(int ocn, int ocd){
	this->ocn = ocn;
	this->ocd = ocd;
	update();
}

void cconv::update(){
	int n = ocn*icd;
	m_d = ocd*icn;
	m_a = n / m_d;
	m_b = n - m_d * m_a;
	m_rem = 0;
	m_rem2 = 0;
}

int cconv::tick1(int ticks){
	int clk = 0;
	while(0<ticks){
		clk += m_a;
		m_rem += m_b;
		while(m_d<=m_rem){
			m_rem -= m_d;
			clk++;
		}
		ticks--;
	}
	return clk;
}
#if 0
int cconv::tick2(int ticks){
	int x = icd * ocn * ticks / (icn * ocd);
	int rxd = icd * ocn * ticks - icn * ocd * x;
	m_rem2 += rxd;
	while((icn*ocd)<=m_rem2){
		m_rem2 -= icn*ocd;
		x++;
	}
	return x;
}
#endif
int cconv::tick2(int ticks){
	__int64 N = icd * ocn;
	__int64 D = icn * ocd;
	__int64 x = N * ticks / D;
	__int64 rxd = N * ticks - D * x;
	m_rem2 += rxd;
	if(D<=m_rem2){
		int rr = m_rem2 / D;
		m_rem2 -= D * rr;
		x += rr;
	}
	return x;
}

int cconv::tick(int ticks){
	return tick2(ticks);
	_c1++;
	int x1 = tick1(ticks);
	int x2 = tick2(ticks);
	int diff = x1 - x2;
	return x2;
}

/*
int cconv::tick(int ticks){
	int clk = 0;
	while(0<ticks){
		clk += m_a;
		m_rem += m_b;
		while(m_d<=m_rem){
			m_rem -= m_d;
			clk++;
		}
		ticks--;
	}
	return clk;
}
*/
