#include "portmidi.h"
#include "porttime.h"
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include "math.h" //i.e. for min() and max() function calls


void midiOutNoteOn(const char* mididevicename, int channelid, int notenumber, int velocity);
void midiOutNoteOff(const char* mididevicename, int channelid, int notenumber);
void midiOutNoteOnNoteOff(const char* mididevicename, int channelid, int notenumber, int duration, int velocity);

void midiOutProgramChange(const char* mididevicename, int channelid, int programnumber);

void midiOutContinuousController(const char* mididevicename, int channelid, int ccnumber, int ccstart, int ccend, int ccstep, int startdelay_ms, int serieduration_ms);
