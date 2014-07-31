
#include <Windows.h>
#include <mmintrin.h>   // MMX
#include <emmintrin.h>  // SSE2
#include <intrin.h>     // SSE3

#include <math.h>
#include "ritzcore.h"
#include "mame/mame_dmy.h"
#include "player.h"
#include "soundbuf.h"
#include "chiptrk.h"

#define PI (3.1415926535897932384626433832795028841971)

chiptrack::chiptrack(__int32 length){
	size_t bytes;
	stream_sample_t** buf;
	//__m128iを使用するので、lengthのサイズを16の倍数に調整する
	length = (length+15) & ~0xf;
	bytes = length * sizeof(signed __int16);
	this->sndbuf = (signed __int16*)AMALLOC(bytes);
	memset(this->sndbuf, 0, bytes);
	effective_samples = 0;
	attenuator = 2;
	gain_l = gain_r = 1.0f;
}

chiptrack::~chiptrack(){
	AFREE(this->sndbuf);
	this->sndbuf = NULL;
	effective_samples = 0;
}

void chiptrack::setPan(float alpha){
    // パンの設定
    // L:-1 R:1 MIDDLE: 0
	if(alpha==-1.0f){ // left
		gain_l = 1.0f;
		gain_r = 0.0f;
	} else if(alpha==0.0f){ // center
		gain_l = 1.0f;
		gain_r = 1.0f;
	} else if(alpha==1.0f){ // right
		gain_l = 0.0f;
		gain_r = 1.0f;
	} else {
		// 1 = L^2 + R^2 とする
		// 0 <= w <= PI/2
		// cos(w): 1 -> 0
		// sin(w): 0 -> 1
		double w = PI * (alpha+1) / 4.0;
		gain_l = (float)cos(w);
		gain_r = (float)sin(w);
	}
}
