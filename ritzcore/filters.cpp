/*
フィルタ
SoundBufferを対象に設計されています。
各チャンネル16ビットのデータがL,R,L,R,...で並んでいる。
 */
#include "ritzcore.h"
#include "filters.h"
#include "soundbuf.h"
#include <mmintrin.h>   // MMX
#include <emmintrin.h>  // SSE2
#include <intrin.h>     // SSE3


FilterBase::FilterBase(int type)
	:magic(0xc0ffee)
	,filter_type(type)
{
}

FilterBase::~FilterBase(){
	//
}

/****************************************************************
 *
 * FIR
 *
 ****************************************************************/

FIRFilter::FIRFilter()
	:FilterBase(1)
{
	coeffs = NULL;
	inpbuf = NULL;
	nTaps = 0;
}

void FIRFilter::setFirCoeffs(int ntaps, float *coeffs){
	if(this->coeffs!=NULL){
		AFREE(this->coeffs);
		this->coeffs = NULL;
	}
	if(inpbuf!=NULL){
		AFREE(inpbuf);
		inpbuf = NULL;
	}
	if(ntaps<=0)
		return;
	// 確保するメモリのサイズを16バイト境界に合わせる
	// アライメントはAMALLOC側で面倒見てください。
	size_t alloc_bytes = sizeof(float) * ntaps;
	alloc_bytes = (alloc_bytes+15) & ~0x0f;
	// 係数保持
	this->coeffs = (float*)AMALLOC(alloc_bytes);
	memcpy(this->coeffs, coeffs, alloc_bytes);
	nTaps = ntaps;
	// 入力バッファ ステレオ分
	inpbuf = (float*)AMALLOC(alloc_bytes*2);
	inpbuf_len = ntaps * 2;
	inpbuf_pos = 0;
	for(int i=0; i<inpbuf_len; i++){
		inpbuf[i] = 0.0f;
	}
}

FIRFilter::~FIRFilter(){
	if(coeffs!=NULL)
		AFREE(coeffs);
	if(inpbuf!=NULL)
		AFREE(inpbuf);
}




void FIRFilter::apply(SoundBuffer* sndbuf){
	if(coeffs==NULL) return;
	_mm_prefetch((const CHAR*)inpbuf, _MM_HINT_T1);
	//_mm_prefetch((const CHAR*)coeffs, _MM_HINT_T1);
	RITZSAMP *ptr = sndbuf->buf;
	int nloop = sndbuf->effective_samples >> 1; // 1ループで2サンプルを扱うから
	while(0<nloop--){
		register float sum_r = 0.0f;
		register float sum_l = 0.0f;
		int idx = inpbuf_pos;
		inpbuf[idx]   = *(ptr);
		inpbuf[idx+1] = *(ptr+1);
		for(int j=0; j<nTaps; j++){
			sum_l += coeffs[j] * inpbuf[idx++];
			sum_r += coeffs[j] * inpbuf[idx++];
			if(inpbuf_len<=idx){
				idx = 0;
			}
		}
		*(ptr++) = (int)sum_l;
		*(ptr++) = (int)sum_r;
		inpbuf_pos += 2;
		if(inpbuf_len<=inpbuf_pos)
			inpbuf_pos = 0;
	}
}
