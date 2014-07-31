#ifndef PLAYER_H_INCLUDED__
#define PLAYER_H_INCLUDED__


class SoundPlayer{
public:
	HWAVEOUT h_waveout;
	WAVEFORMATEX wfx;
	SoundPlayer();
	~SoundPlayer();

};


#endif /* PLAYER_H_INCLUDED__ */