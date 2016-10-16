#include "midifilereader.h"

using namespace std;

MidiFileReader::MidiFileReader(const char *filePath): _rawData("") {
    std::locale::global(std::locale(""));
    ifstream ifs(filePath, ios::binary | ios::in | ios::ate);
    if(ifs) {
        auto fileSize = ifs.tellg();
        ifs.seekg(0);
        _rawData.reserve(fileSize);
        while(!ifs.eof()) {
            _rawData += ifs.get();
        }
    }
}

MidiEvent MidiFileReader::readEvent(Stream &stream) {
    MidiEvent event;
    stream.setOutputBuffer(&event.rawData);
    event.deltaTime = stream.readVarInt();
    unsigned char eventTypeByte = stream.readInt8();
    if ((eventTypeByte & 0xf0) == 0xf0) {
        if (eventTypeByte == 0xff) {
            event.type = MidiEvent::META;
            unsigned char subtypeByte = stream.readInt8();
            unsigned long long length = stream.readVarInt();
            event.length = length;
            switch(subtypeByte) {
                case 0x00:
                    event.subtype = MidiEvent::SEQ_NUM;
                    if (length != 2) throw "Expected length for sequenceNumber event is 2, got ";
                    event.number = stream.readInt16();
                    break;
                case 0x01:
                    event.subtype = MidiEvent::TEXT;
                    event.text = stream.read(length);
                    break;
                case 0x02:
                    event.subtype = MidiEvent::COPYRIGHT;
                    event.text = stream.read(length);
                    break;
                case 0x03:
                    event.subtype = MidiEvent::TRACK_NAME;
                    event.text = stream.read(length);
                    break;
                case 0x04:
                    event.subtype = MidiEvent::INS_NAME;
                    event.text = stream.read(length);
                    break;
                case 0x05:
                    event.subtype = MidiEvent::LYRICS;
                    event.text = stream.read(length);
                    break;
                case 0x06:
                    event.subtype = MidiEvent::MARKER;
                    event.text = stream.read(length);
                    break;
                case 0x07:
                    event.subtype = MidiEvent::CUE_POINT;
                    event.text = stream.read(length);
                    break;
                case 0x20:
                    event.subtype = MidiEvent::CHANNEL_PREFIX;
                    if (length != 1) throw "Expected length for midiChannelPrefix event is 1, got ";
                    event.channel = stream.readInt8();
                    break;
                case 0x2f:
                    event.subtype = MidiEvent::EOT;
                    if (length != 0) throw "Expected length for endOfTrack event is 0, got ";
                    break;
                case 0x51:
                    event.subtype = MidiEvent::SET_TEMPO;
                    if (length != 3) throw "Expected length for setTempo event is 3, got ";
                    event.microsecondsPerBeat = (stream.readInt8() << 16) | (stream.readInt8() << 8) | stream.readInt8();
                    break;
                case 0x54: {
                    event.subtype = MidiEvent::SMPTE_OFFSET;
                    if (length != 5) throw "Expected length for smpteOffset event is 5, got ";
                    unsigned char hourByte = stream.readInt8();
                    switch(hourByte & 0x60) {
                    case 0x00:
                        event.frameRate = 24;
                        break;
                    case 0x20:
                        event.frameRate = 25;
                        break;
                    case 0x40:
                        event.frameRate = 29;
                        break;
                    case 0x60:
                        event.frameRate = 30;
                        break;
                    default:
                        throw "unknown hourByte.";
                    }
                    event.hour = hourByte & 0x1f;
                    event.min = stream.readInt8();
                    event.sec = stream.readInt8();
                    event.frame = stream.readInt8();
                    event.subframe = stream.readInt8();
                    break;
                }
                case 0x58:
                    event.subtype = MidiEvent::TIME_SIGN;
                    if (length != 4) throw "Expected length for timeSignature event is 4, got ";
                    event.numerator = stream.readInt8();
                    event.denominator = pow(2, stream.readInt8());
                    event.metronome = stream.readInt8();
                    event.thirtyseconds = stream.readInt8();
                    break;
                case 0x59:
                    event.subtype = MidiEvent::KEY_SIGN;
                    if (length != 2) throw "Expected length for keySignature event is 2, got ";
                    event.key = stream.readSInt8();
                    event.scale = stream.readInt8();
                    break;
                case 0x7f:
                    event.subtype = MidiEvent::SEQ_SPEC;
                    event.data = stream.read(length);
                    break;
                default:
                    // console.log("Unrecognised meta event subtype: " + subtypeByte);
                    event.subtype = MidiEvent::UNKNOWN;
                    event.data = stream.read(length);
            }
        } else if (eventTypeByte == 0xf0) {
            event.type = MidiEvent::SYSEX;
            event.length = stream.readVarInt();
            event.data = stream.read(event.length);
        } else if (eventTypeByte == 0xf7) {
            event.type = MidiEvent::DIV_SYSEX;
            event.length = stream.readVarInt();
            event.data = stream.read(event.length);
        } else {
            throw "Unrecognised MIDI event type byte: " + eventTypeByte;
        }
    } else {
        /* channel event */
        unsigned char param1;
        if ((eventTypeByte & 0x80) == 0) {
            /* running status - reuse lastEventTypeByte as the event type.
             eventTypeByte is actually the first parameter
             */
            param1 = eventTypeByte;
            eventTypeByte = lastEventTypeByte;
        } else {
            param1 = stream.readInt8();
            lastEventTypeByte = eventTypeByte;
        }
        unsigned char eventType = eventTypeByte >> 4;
        event.channel = eventTypeByte & 0x0f;
        event.type = MidiEvent::CHANNEL;
        switch (eventType) {
            case 0x08:
                event.subtype = MidiEvent::NOTE_ON;
                event.noteNumber = param1;
                event.velocity = stream.readInt8();
                break;
            case 0x09:
                event.noteNumber = param1;
                event.velocity = stream.readInt8();
                if (event.velocity == 0) {
                    event.subtype = MidiEvent::NOTE_OFF;
                } else {
                    event.subtype = MidiEvent::NOTE_ON;
                }
                break;
            case 0x0a:
                event.subtype = MidiEvent::NOTE_AFTERTOUCH;
                event.noteNumber = param1;
                event.amount = stream.readInt8();
                break;
            case 0x0b:
                event.subtype = MidiEvent::CONTROLLER;
                event.controllerType = param1;
                event.value = stream.readInt8();
                break;
            case 0x0c:
                event.subtype = MidiEvent::PROG_CHANGE;
                event.programNumber = param1;
                break;
            case 0x0d:
                event.subtype = MidiEvent::CHAN_AFTERTOUCH;
                event.amount = param1;
                break;
            case 0x0e:
                event.subtype = MidiEvent::PITCH;
                event.value = param1 + (stream.readInt8() << 7);
                break;
            default:
                throw "Unrecognised MIDI event type: " + eventType;
        }
    }
    stream.setOutputBuffer();
    return event;
}


void MidiFileReader::readChunk(Stream &stream, std::string &id, std::string &data) {
    stream.setOutputBuffer();
    id = stream.read(4);
    unsigned long length = stream.readInt32();
    data = stream.read(length);
}

void MidiFileReader::load(MidiData &midiData) {
    Stream stream(_rawData);
    if(stream.length() == 0) throw "MidiFileReader::load() - No data.";

    std::vector<std::vector<MidiEvent>> &tracks = midiData.tracks;
    MidiHeader &result = midiData.header;
    std::string id;
    std::string tmpData;

    readChunk(stream, id, tmpData);
    if(id != "MThd" || tmpData.length() != 6) {
        throw "MidiFileReader::load() - Not a valid MIDI file.";
    }

    Stream headerStream(tmpData);
    result.formatType = headerStream.readInt16();
    result.trackCount = headerStream.readInt16();
    result.ticksPerBeat = headerStream.readInt16();
    if(result.ticksPerBeat & 0x8000) {
        throw "Expressing time division in SMTPE frames is not supported yet";
    }

    tracks.clear();
    tracks.resize(result.trackCount);
    for(int i=0; i<result.trackCount; i++) {
        readChunk(stream, id, tmpData);
        if(id != "MTrk") {
            throw "MidiFileReader::load() - Expected MTrk not found.";
        }
        Stream trackStream(tmpData);
        while(trackStream.length()) {
            tracks[i].emplace_back(readEvent(trackStream));
        }
    }

    midiData.refresh();
}


void MidiData::calcLastTimeOfEvents() {
    MidiData &self = *this;
    std::vector<std::vector<MidiEvent>> &tracks = self.tracks;
    int trackNum = tracks.size();
#define noteIdx(channel, note) (((channel)<<8)|(note))
    unordered_map<unsigned int, pair<int, int>> lastNoteOnEvtIdx;
    self.totalTicks = 0;
    for(int i=0; i<trackNum; i++) {
        unsigned long absoluteTicks = 0;
        int evtNum = tracks[i].size();
        for(int j=0; j<evtNum; j++) {
            MidiEvent &event = tracks[i][j];
            absoluteTicks += event.deltaTime;
            event.absoluteTicks = absoluteTicks;

            if(event.subtype == MidiEvent::NOTE_ON) {
                unsigned int idx = noteIdx(event.channel, event.noteNumber);
                pair<int, int> lastInfo = lastNoteOnEvtIdx[idx];
                if(lastInfo.second != 0) {
                    MidiEvent &prevEvt = tracks[lastInfo.first][lastInfo.second];
                    prevEvt.lastTime = absoluteTicks - prevEvt.absoluteTicks;
                }
                lastNoteOnEvtIdx[idx] = pair<int, int>(i, j);
            } else if(event.subtype == MidiEvent::NOTE_OFF) {
                unsigned int idx = noteIdx(event.channel, event.noteNumber);
                pair<int, int> lastInfo = lastNoteOnEvtIdx[idx];
                if(lastInfo.second != 0) {
                    MidiEvent &prevEvt = tracks[lastInfo.first][lastInfo.second];
                    prevEvt.lastTime = absoluteTicks - prevEvt.absoluteTicks;
                    lastNoteOnEvtIdx.erase(idx);
                }
            }
        }
        self.totalTicks = max(self.totalTicks, absoluteTicks);
    }
    for(auto &p: lastNoteOnEvtIdx) {
        auto &info = p.second;
        if(info.second) {
            MidiEvent &prevEvt = tracks[info.first][info.second];
            prevEvt.lastTime = -1;
        }
    }
#undef noteIdx
}

void MidiData::calcAbsoluteTime() {
    vector<int> currentEventAtTrackId;
    currentEventAtTrackId.resize(this->header.trackCount, -1);
    double absoluteTime = 0.0;
    bool finishFlag = false;
    double beatsPerMinute = 120.0;
    int ticksPerBeat = this->header.ticksPerBeat;
    unsigned long long absoluteTicks = 0;
    int trackNum = this->header.trackCount;
#define ticksToMs(ticks) ((60000.0 * (ticks)) / (ticksPerBeat * beatsPerMinute))
    while(!finishFlag) {
        unsigned long long deltatime = 999999999L;
        finishFlag = true;
        for(int i=0; i<trackNum; i++) {
            if(currentEventAtTrackId[i]+1 >= tracks[i].size()) continue;
            finishFlag = false;
            MidiEvent &event = tracks[i][currentEventAtTrackId[i]+1];
            deltatime = min(deltatime, event.absoluteTicks - absoluteTicks);
        }
        if(finishFlag) break;
        absoluteTicks += deltatime;
        absoluteTime += ticksToMs(deltatime);
        for(int i=0; i<trackNum; i++) {
            if(currentEventAtTrackId[i]+1 >= tracks[i].size()) continue;
            MidiEvent &event = tracks[i][currentEventAtTrackId[i]+1];
            if(event.absoluteTicks == absoluteTicks) {
                currentEventAtTrackId[i] += 1;
                event.absoluteTime = (unsigned long long)absoluteTime;
                if(event.subtype == MidiEvent::SET_TEMPO) {
                    beatsPerMinute = 60000000.0 / (event.microsecondsPerBeat*1.0);
                    setTempoEvent.emplace_back(event);
                }
            }
        }
    }
    totalTime = absoluteTime;
#undef ticksToMs
}
