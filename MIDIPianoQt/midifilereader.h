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
    unsigned int trackId;
    long lastEventIdx = -1;
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

    class iterator {
        std::vector<size_t> currentEventAtTrackId;
        long long absTicks;
    public:
        friend struct MidiData;

        typedef MidiData T;
        typedef iterator self;
        typedef T value_type;
        typedef MidiEvent *pointer;
        typedef MidiEvent &reference;
        typedef size_t size_type;
        T *data;


        iterator(T *x): data(x), absTicks(-1) {
            currentEventAtTrackId.resize(data->tracks.size(), 0);
            for(size_t i=0; i<currentEventAtTrackId.size(); i++) {
                if(currentEventAtTrackId[i] >= data->tracks[i].size()) continue;
                MidiEvent &event = data->tracks[i][currentEventAtTrackId[i]];
                if((long long)event.absoluteTicks < absTicks || absTicks == -1) absTicks = event.absoluteTicks;
            }
        }
        iterator(): data(NULL), absTicks(-1) { }

        bool operator==(const self& x) const {
            if(data != x.data) return false;
            //if(currentEventAtTrackId.size() != x.currentEventAtTrackId.size())
            //    throw "Same midiData with different trackNum in iterator.";
            if(absTicks != x.absTicks) return false;
            int n = currentEventAtTrackId.size();
            for(int i=0; i<n; i++) if(currentEventAtTrackId[i] != x.currentEventAtTrackId[i]) return false;
            return true;
        }
        bool operator!=(const self& x) const { return !(operator==(x)); }

        reference operator*() {
            for(size_t i=0; i<currentEventAtTrackId.size(); i++) {
                if(currentEventAtTrackId[i] >= data->tracks[i].size()) continue;
                MidiEvent &event = data->tracks[i][currentEventAtTrackId[i]];
                if(event.absoluteTicks == absTicks) return event;
            }
            throw "MidiData::iterator::operator*() - Error.";
        }
        pointer operator->() { return &(operator*()); }

        self& operator++() {
            bool finishFlag = true, notFoundFlag = true;
            for(size_t i=0; i<currentEventAtTrackId.size(); i++) {
                if(currentEventAtTrackId[i] >= data->tracks[i].size()) continue;
                finishFlag = false;
                MidiEvent &event = data->tracks[i][currentEventAtTrackId[i]];
                if(event.absoluteTicks == absTicks) {
                    currentEventAtTrackId[i]++;
                    notFoundFlag = false;
                    break;
                }
            }
            if(finishFlag || notFoundFlag) throw "MidiData::iterator::operator++() - Error.";
            absTicks = 1999999999;
            finishFlag = true;
            for(size_t i=0; i<currentEventAtTrackId.size(); i++) {
                if(currentEventAtTrackId[i] >=  data->tracks[i].size()) continue;
                finishFlag = false;
                MidiEvent &event = data->tracks[i][currentEventAtTrackId[i]];
                if((long long)event.absoluteTicks < absTicks) {
                    absTicks = (long long)(event.absoluteTicks);
                }
            }
            if(finishFlag) {
                absTicks = -1;
            }
            return (*this);
        }
    };

    iterator begin() {
        return iterator(this);
    }

    iterator end() {
        iterator res(this);
        for(size_t i=0; i<tracks.size(); i++) {
            res.currentEventAtTrackId[i] = tracks[i].size();
        }
        res.absTicks = -1;
        return res;
    }

    iterator find(unsigned long time) {
        iterator res(this);
        res.absTicks = 1999999999;
        for(int i=0; i<tracks.size(); i++) {
            long idx = binarySearch(time, tracks[i], 0, tracks[i].size()-1);
            if(idx == -1) idx = tracks[i].size();
            else {
                unsigned long absTicks = tracks[i][idx].absoluteTicks;
                if(absTicks < res.absTicks)
                    res.absTicks = absTicks;
            }
            res.currentEventAtTrackId[i] = idx;
        }
        return res;
    }

private:
    void calcLastTimeOfEvents();
    void calcAbsoluteTime();

    // find the smallest event that event.time >= v
    // return -1 means that no next evt
    long binarySearch(unsigned long v, std::vector<MidiEvent> &track, size_t l, size_t r) {
#define CRITERIA(idx) (track[idx].absoluteTime >= v)
        if(l == r) {
            if(CRITERIA(l)) return l;
            else return -1;
        }
        if(l == r-1) {
            if(CRITERIA(l)) return l;
            else return CRITERIA(r) ? r : -1;
        }
        size_t m = (l+r) >> 1;
        if(CRITERIA(m)) {
            long tmp = binarySearch(v, track, l, m-1);
            return tmp == -1 ? m : tmp;
        } else return binarySearch(v, track, m+1, r);
#undef CRITERIA
    }
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
