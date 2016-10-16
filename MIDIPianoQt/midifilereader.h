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
        static std::string _dummy;
        std::string &buffer;
        unsigned long long p, n;

        void getRaw(unsigned long long length, bool rewind=true) {
            if(!hasOutputBuffer()) return;
            assert(p < n);
            buffer += rawData.substr(p, length);
            if(!rewind) p += length;
        }

        std::string &_tmp;
        void outputBegin(std::string &newBuffer) {
            _tmp = buffer;
            buffer = newBuffer;
        }
        void outputEnd() {
            buffer = _tmp;
        }

    public:
        Stream(const std::string &_rawData):
            rawData(_rawData),
            buffer(_dummy), _tmp(_dummy), p(0), n(_rawData.length())
        {}

        Stream operator = (const Stream &stream) {
            Stream res(stream.rawData);
            res.p = stream.p;
            res.buffer = stream.buffer;
            res._tmp = stream._tmp;
            return res;
        }

        inline bool hasOutputBuffer() const {
            return (&_dummy) == (&buffer);
        }

        void setOutputBuffer(std::string &_buffer = _dummy) {
            buffer = _buffer;
        }

        std::string read(unsigned long long length) {
            getRaw(length);
            std::string res;
            outputBegin(res);
            getRaw(length, false);
            outputEnd();
            return res;
        }

        long readInt32() {
            auto tmp = read(4);
            return (tmp[0] << 24) | (tmp[1] << 16) | (tmp[2] << 8) | tmp[3];
        }

        int readInt16() {
            auto tmp = read(2);
            return (tmp[0] << 8) | tmp[1];
        }

        unsigned char readInt8() {
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
    MidiFileReader(std::string filePath);
    ~MidiFileReader() {}
    MidiEvent readEvent(Stream &stream);
    void load(MidiData &midiData);
};

#endif // MIDIFILEREADER_H
