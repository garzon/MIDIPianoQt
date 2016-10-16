#ifndef MYMIDI_H
#define MYMIDI_H

#include <windows.h>
#include <mmsystem.h>
#include <functional>

class myMIDI{
private:
	HMIDIIN _inHandle;
	HMIDIOUT _outHandle;
	void CALLBACK midiCallback(UINT uMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2);
public:
	class midiOutError{};
	class midiOpened{};
	class midiNotOpened{}; 
	myMIDI(std::function<void(long,long,long,long)> *callback,UINT inDevice=0,UINT outDevice=-1);
	bool inputValid() const;
	bool outputValid() const;
	MMRESULT sendMsg(unsigned char vol,unsigned char note,unsigned char evt,unsigned char channel=0) const;
	~myMIDI();
};

namespace _myMIDI{

	static bool _isCreated=false;
	static std::function<void(long,long,long,long)> *_myCallback;
	typedef void (CALLBACK myMIDI::*_callbackFunctionType)(UINT,DWORD,DWORD,DWORD);
	typedef union{
		_callbackFunctionType function;
		DWORD_PTR address;
	} functionPointer;

};

#endif