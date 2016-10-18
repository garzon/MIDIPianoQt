#include "MidiIOManager.h"

static bool _isCreated = false;
static std::function<void(long,long,long,long)> *_myCallback;
typedef void (CALLBACK MidiIOManager::*_callbackFunctionType)(UINT,DWORD,DWORD,DWORD);
typedef union{
    _callbackFunctionType function;
    DWORD_PTR address;
} functionPointer;

static MidiIOManager *_ioManagerInstance;

MidiIOManager *MidiIOManager::getInstance() {
    return _ioManagerInstance;
}

void CALLBACK MidiIOManager::midiCallback(UINT uMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2){
    //qDebug() << QString().sprintf("%x %x %x %x", uMsg, dwInstance, dwParam1, dwParam2);
    if(uMsg == MIM_DATA){
        if((dwParam1 & 0xF0) == 0x90){
            DWORD vol = dwParam1 & 0xFF0000;
            dwParam1 += 0x00400000 + vol*2;
            if((dwParam1 & 0xFFFF0000)>=0x800000) dwParam1 = (dwParam1 & 0xFFFF) + 0x7F0000;
		}
        unsigned long vol = dwParam1  & 0xFF0000;
        unsigned long note = dwParam1 & 0x00FF00;
        unsigned long evt = dwParam1  & 0x0000F0;
        unsigned long chn = dwParam1  & 0x00000F;
        vol >>= 16;
        note >>= 8;
        evt >>= 4;
        std::function<void(long,long,long,long)> tmp(*_myCallback);
        tmp(vol, note, evt, chn);
	}
    if(uMsg == MIM_LONGDATA) {
        // not implemented.
	}
}

MidiIOManager::MidiIOManager(std::function<void(long,long,long,long)> *callback,UINT inDevice,UINT outDevice){
    if(_isCreated) throw midiOpened();
    _isCreated = true;
    _ioManagerInstance = this;

    // handle midi input device
    _myCallback = callback;
    functionPointer tmp;
    tmp.function = &MidiIOManager::midiCallback;
    DWORD_PTR pointerValue = tmp.address;
    long result = midiInOpen(&_inHandle, inDevice, pointerValue, NULL, CALLBACK_FUNCTION);
	if(result)
        _inHandle = NULL;
	else
		midiInStart(_inHandle);

    // open output device
    result = midiOutOpen(&_outHandle,outDevice,0,0,0);
	if(result){
        if(_inHandle != NULL){
			midiInStop(_inHandle);
			midiInReset(_inHandle);
			midiInClose(_inHandle);
		}
        _outHandle = NULL;
		throw midiOutError();
	}
    if(_inHandle == NULL){
        //throw midiInError();
	}
}

bool MidiIOManager::inputValid() const { return _inHandle != NULL; }

bool MidiIOManager::outputValid() const { return _outHandle != NULL; }

MMRESULT MidiIOManager::sendMsg(unsigned char vol,unsigned char note,unsigned char evt,unsigned char channel) const{
    if(_outHandle != NULL)
        return midiOutShortMsg(_outHandle,(vol << 16)|(note << 8)|(evt << 4)|channel);
	else throw midiNotOpened();
}

MidiIOManager::~MidiIOManager(){
    if(_inHandle != NULL){
		midiInStop(_inHandle);
		midiInReset(_inHandle);
		midiInClose(_inHandle);
	}
    if(_outHandle != NULL)
		midiOutClose(_outHandle);
    _isCreated = false;
    _ioManagerInstance = NULL;
}

