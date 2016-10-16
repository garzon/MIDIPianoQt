#ifndef MIDICONTROLLER_H
#define MIDICONTROLLER_H

#include "midifilereader.h"

class MidiController
{
    MidiData &midiData;
public:
    MidiController(MidiData &_midiData);
};

#endif // MIDICONTROLLER_H
