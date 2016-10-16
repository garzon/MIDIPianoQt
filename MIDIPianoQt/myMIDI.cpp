#include "myMIDI.h"
#include <cstdio>
#include <qdebug.h>
#include <qstring.h>
void CALLBACK myMIDI::midiCallback(UINT uMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2){
	qDebug() << QString().sprintf("%x %x %x %x", uMsg, dwInstance, dwParam1, dwParam2);
	if(uMsg==MIM_DATA){
		if((dwParam1 & 0xF0)==0x90){
			DWORD vol=dwParam1 & 0xFF0000;
			dwParam1+=0x00400000+vol*2;
			if((dwParam1 & 0xFFFF0000)>=0x800000) dwParam1=(dwParam1 & 0xFFFF) + 0x7F0000;
		}
		long vol=dwParam1  & 0xFF0000;
		long note=dwParam1 & 0x00FF00;
		long evt=dwParam1  & 0x0000F0;
		long chn=dwParam1  & 0x00000F;
		vol/=0x10000;
		note/=0x100;
		evt/=0x10;
		std::function<void(long,long,long,long)> tmp(*_myMIDI::_myCallback);
		tmp(vol,note,evt,chn);
	}
	if (uMsg == MIM_LONGDATA) {
		
	}
}

myMIDI::myMIDI(std::function<void(long,long,long,long)> *callback,UINT inDevice,UINT outDevice){
	if(_myMIDI::_isCreated) throw midiOpened();
	_myMIDI::_isCreated=true;
	_myMIDI::_myCallback=callback;
	_myMIDI::functionPointer tmp;
	tmp.function=&myMIDI::midiCallback;
	DWORD_PTR pointerValue=tmp.address;
	long result=midiInOpen(&_inHandle, inDevice, pointerValue, NULL, CALLBACK_FUNCTION);
	if(result)
		_inHandle=NULL;
	else
		midiInStart(_inHandle);
	result=midiOutOpen(&_outHandle,outDevice,0,0,0);
	if(result){
		if(_inHandle!=NULL){
			midiInStop(_inHandle);
			midiInReset(_inHandle);
			midiInClose(_inHandle);
		}
		_outHandle=NULL;
		throw midiOutError();
	}
	if(_inHandle==NULL){
		// throw midiInError();
	}
}

bool myMIDI::inputValid() const { return (_inHandle!=NULL); }

bool myMIDI::outputValid() const { return (_outHandle!=NULL); }

MMRESULT myMIDI::sendMsg(unsigned char vol,unsigned char note,unsigned char evt,unsigned char channel) const{
	if(_outHandle!=NULL)
		return midiOutShortMsg(_outHandle,vol*0x10000+note*0x100+evt*0x10+channel);
	else throw midiNotOpened();
}

myMIDI::~myMIDI(){
	if(_inHandle!=NULL){
		midiInStop(_inHandle);
		midiInReset(_inHandle);
		midiInClose(_inHandle);
	}
	if(_outHandle!=NULL)
		midiOutClose(_outHandle);
	_myMIDI::_isCreated=false;
}

