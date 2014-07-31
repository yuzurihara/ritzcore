#ifndef BUFFER_H_INCLUDED__
#define BUFFER_H_INCLUDED__

/*** �Đ��o�b�t�@
 *
 *
 *
 */

class SoundBuffer{
private:
	int dummy;
public:
#ifdef _DEBUG
	int __MAGIC__;
#endif
	RITZSAMP *const buf;
	// �o�b�t�@�L���p�V�e�B(�T���v����)
	const UINT32 samples;
	const UINT32 buffer_size; //in bytes.
	// �o�b�t�@���̗L���T���v����
	UINT32 effective_samples;
	HWAVEOUT hwaveout;
	WAVEHDR wavehdr;
public:
	SoundBuffer(UINT32 size)
	:	samples(size),
		buffer_size(sizeof(RITZSAMP)*size),
		buf((RITZSAMP*)AMALLOC(sizeof(RITZSAMP)*size))
	{
		// buf = (SAMP*)GlobalAlloc(bytes);
		wavehdr.lpData = (LPSTR)buf;
		wavehdr.dwBufferLength = buffer_size;
		wavehdr.dwFlags = 0;
		wavehdr.dwUser = 0;
		hwaveout = 0;
		effective_samples = 0;
	}
	~SoundBuffer(){
		if(hwaveout!=0){
			CloseHandle((HANDLE)wavehdr.dwUser);
			waveOutUnprepareHeader(hwaveout, &wavehdr, sizeof(WAVEHDR));
		}
		//free(buf);
		AFREE(buf);
	}
	int setPlayable(HWAVEOUT hwaveout){
		if(hwaveout==0)
			return -1;
		this->hwaveout = hwaveout;
		DWORD tmp_bytes = wavehdr.dwBufferLength;
		wavehdr.dwBufferLength = buffer_size;
		wavehdr.dwUser = (DWORD)CreateEvent(NULL, TRUE, TRUE, NULL);
		waveOutPrepareHeader(hwaveout, &wavehdr, sizeof(WAVEHDR));
		wavehdr.dwBufferLength = tmp_bytes;
		return 0;
	}
	UINT32 getBufferLength(){
		// �L���ȃf�[�^�����o�C�g���ŕԂ�
		return wavehdr.dwBufferLength;
	}
	void clear(){
		effective_samples = 0;
		memset(buf, 0, wavehdr.dwBufferLength);
	}
};

#endif /* BUFFER_H_INCLUDED__ */
