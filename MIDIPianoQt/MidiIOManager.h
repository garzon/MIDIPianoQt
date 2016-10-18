#ifndef MYMIDI_H
#define MYMIDI_H

#include <windows.h>
#include <mmsystem.h>
#include <functional>

class MidiIOManager{
private:
	HMIDIIN _inHandle;
	HMIDIOUT _outHandle;
	void CALLBACK midiCallback(UINT uMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2);

public:
    class midiInError{};
	class midiOutError{};
	class midiOpened{};
    class midiNotOpened{};

    MidiIOManager(std::function<void(long,long,long,long)> *callback, UINT inDevice=0, UINT outDevice=-1);
	bool inputValid() const;
	bool outputValid() const;
    MMRESULT sendMsg(unsigned char vol, unsigned char note, unsigned char evt, unsigned char channel=0) const;
    ~MidiIOManager();

    static MidiIOManager *getInstance();
};

#endif
