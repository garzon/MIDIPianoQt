#ifndef MIDIFILEREADER_H
#define MIDIFILEREADER_H

#include <iostream>
#include <fstream>
#include <string>
#include <assert.h>
#include <cstring>
#include <vector>
#include <unordered_map>
#include <algorithm>

struct MidiEvent {
    unsigned long long deltaTime; // deltaTicks in fact
    unsigned long long absoluteTicks;
    unsigned long long absoluteTime;
    unsigned long long lastTime;  // lastTicks in fact
    enum {
        META, SYSEX, DIV_SYSEX, CHANNEL
    } type;
    enum {
        SEQ_NUM, TEXT, COPYRIGHT, TRACK_NAME, INS_NAME, LYRICS,
        MARKER, CUE_POINT, CHANNEL_PREFIX, EOT, SET_TEMPO, SMPTE_OFFSET,
        TIME_SIGN, KEY_SIGN, SEQ_SPEC, UNKNOWN, NOTE_OFF, NOTE_ON,
        NOTE_AFTERTOUCH, CONTROLLER, PROG_CHANGE, CHAN_AFTERTOUCH, PITCH
    } subtype;
    std::string rawData;
    //union {
        int number;
        unsigned char channel;
        unsigned long microsecondsPerBeat;
        //struct {
            unsigned char frameRate;
            unsigned char hour;
            unsigned char min;
            unsigned char sec;
            unsigned char frame;
            unsigned char subframe;
        //};
        //struct {
            unsigned char numerator;
            unsigned long denominator;
            unsigned char metronome;
            unsigned char thirtyseconds;
        //};
        //struct {
            char key;
            unsigned char scale;
        //};
        //struct {
        //    union {
                std::string data;
                std::string text;
        //    };
            unsigned long long length;
        //};
        //struct {
            unsigned char noteNumber;
        //    union {
                unsigned char velocity;
                unsigned char amount;
        //    };
        //};
        //struct {
            unsigned char controllerType;
            unsigned char value;
        //};
        unsigned char programNumber;
    //};
    //MidiEvent() {}
    //~MidiEvent() {}
    //MidiEvent(MidiEvent &&event) {
    //    memcpy(this, &event, sizeof(event)); // shallow copy?
    //}
};

struct MidiHeader {
    int formatType;
    int trackCount;
    int ticksPerBeat;
};

struct MidiData {
    MidiHeader header;
    std::vector<std::vector<MidiEvent>> tracks;
    std::vector<MidiEvent> setTempoEvent;
    unsigned long totalTime;
    unsigned long totalTicks;
    void refresh() {
        calcLastTimeOfEvents();
        calcAbsoluteTime();
    }
private:
    void calcLastTimeOfEvents();
    void calcAbsoluteTime();
};

class MidiFileReader {
    class Stream {
        const std::string &rawData;
        std::string *buffer;
        unsigned long long p, n;

    public:
        Stream(const std::string &_rawData):
            rawData(_rawData),
            buffer(NULL), p(0), n(_rawData.length())
        {}

        Stream operator = (const Stream &stream) {
            Stream res(stream.rawData);
            res.p = stream.p;
            res.buffer = stream.buffer;
            return res;
        }

        void setOutputBuffer(std::string *_buffer = NULL) {
            buffer = _buffer;
        }

        std::string read(unsigned long long length) {
            if(length+p > n) throw "Stream::read - not in valid range.";
            std::string res = rawData.substr(p, length);
            if(buffer != NULL)
                (*buffer) += res;
            p += length;
            return res;
        }

        inline long readInt32() {
            unsigned long tmp = readInt16();
            unsigned long tmp2 = readInt16();
            return (tmp << 16) | tmp2;
        }

        inline int readInt16() {
            unsigned int tmp = readInt8();
            unsigned int tmp2 = readInt8();
            return (tmp << 8) | tmp2;
        }

        inline unsigned char readInt8() {
            return (unsigned char)(read(1)[0]);
        }

        char readSInt8() {
            return read(1)[0];
        }

        unsigned long long readVarInt() {
            unsigned long long res = 0;
            while(true) {
                unsigned char b = readInt8();
                if(b & 0x80) {
                    res += (b & 0x7f);
                    res <<= 7;
                } else {
                    res += b;
                    break;
                }
            }
            if(((long long)(res)) < 0) throw std::exception("readVarInt overflow.");
            return res;
        }

        unsigned long long length() const {
            return n-p;
        }
    };

    static unsigned long pow(unsigned long base, int times) {
        unsigned long res = 1;
        for(int i=0; i<times; i++) {
            res *= base;
        }
        return res;
    }

    std::string _rawData;
    unsigned char lastEventTypeByte;

    void readChunk(Stream &stream, std::string &id, std::string &data);
public:
    MidiFileReader(const char *filePath);
    ~MidiFileReader() {}
    MidiEvent readEvent(Stream &stream);
    void load(MidiData &midiData);
};

#endif // MIDIFILEREADER_H
