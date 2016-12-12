#include "spimidilib.h"

#define INPUT_BUFFER_SIZE 100
#define OUTPUT_BUFFER_SIZE 0
#define DRIVER_INFO NULL
#define TIME_PROC ((int32_t (*)(void *)) Pt_Time)
#define TIME_INFO NULL
#define TIME_START Pt_Start(1, 0, 0) /* timer started w/millisecond accuracy */

#define STRING_MAX 80 /* used for console input */

int32_t global_latency = 0;

void midinote(const char* devicename, int channelid, int notenumber, int duration, int velocity); 
void midiprogramchange(const char* mididevicename, int channelid, int programnumber); 
void midicontinuouscontroller(const char* mididevicename, int channelid, int ccnumber, int ccstart, int ccend, int ccstep, int startdelay_ms, int serieduration_ms); 

/*
;;midiOutNoteOn(MidiDevice,Channel,Note,NoteVel)
;;midiOutNoteOff(MidiDevice,Channel,Note,NoteVel)
;;midiOutNoteOnNoteOff(MidiDevice,Channel,Note,NoteDur,NoteVel)
;;midiOutProgramChange(MidiDevice,Channel,Program)
;;midiOutContinuousController(MidiDevice, Channel, ccNumber, ccStart, ccEnd, ccStep, StartDelay, SeriesDuration)
*/

void midiOutNoteOn(const char* mididevicename, int channelid, int notenumber, int velocity)
{
	int duration=0;
	midinote(mididevicename, channelid, notenumber, duration, velocity);
}

void midiOutNoteOff(const char* mididevicename, int channelid, int notenumber)
{
	int duration=0;
	int velocity=0;
	midinote(mididevicename, channelid, notenumber, duration, velocity);
}

void midiOutNoteOnNoteOff(const char* mididevicename, int channelid, int notenumber, int duration, int velocity)
{
	midinote(mididevicename, channelid, notenumber, duration, velocity);
}

void midiOutProgramChange(const char* mididevicename, int channelid, int programnumber)
{
	midiprogramchange(mididevicename, channelid, programnumber); 
}

void midiOutContinuousController(const char* mididevicename, int channelid, int ccnumber, int ccstart, int ccend, int ccstep, int startdelay_ms, int serieduration_ms)
{
	midicontinuouscontroller(mididevicename, channelid, ccnumber, ccstart, ccend, ccstep, startdelay_ms, serieduration_ms);
}



// if caller alloc devicename like this: char devicename[STRING_MAX]; 
//	then parameters are devicename and STRING_MAX respectively       
int get_device_id(const char* devicename, int devicename_buffersize)
{
	int deviceid=-1;
	int i;
    for (i=0; i<Pm_CountDevices(); i++) 
	{
        const PmDeviceInfo* info = Pm_GetDeviceInfo(i);
 		if(strcmp(info->name, devicename)==0) 
		{
			deviceid=i;
			break;
		}
    }
	#ifdef DEBUG
		if(deviceid==-1)
		{
			printf("get_device_id(), did not find any matching deviceid\n");
		}
	#endif //DEBUG
	return deviceid;
}


//the purpose of creating nostream() version was to attempt to prevent cc 64 values being sent on all channels when closing mididevice
//by relying on Pm_WriteShort to send midi message instead of Pm_Write to send midi event (a midi message with timestamp) but did not
//find a way to send midi message without timestamp other than by specifying zero latency when opening the mididevice which provoques
//timestamps to be ignored later on when sending mesage onto this device and thereby prevents cc 64 value 0 to be sent on all channels
//upon closing mididevice.
void midinote_nostream(int latency, int deviceid, int close_mididevice_onexit, int channelid, int notenumber, int duration, int velocity) 
{
    PmStream* midi;
	char line[80];
    PmEvent buffer[3];

	/*
    // It is recommended to start timer before PortMidi 
    TIME_START;
	*/

	/* open output device */
    Pm_OpenOutput(&midi, 
                  deviceid, 
                  DRIVER_INFO,
                  OUTPUT_BUFFER_SIZE, 
                  TIME_PROC,
                  TIME_INFO, 
                  latency);
#ifdef DEBUG
    printf("Midi Output opened with %ld ms latency.\n", (long) latency);
#endif //DEBUG

    /* writing midi for immediate output, we use timestamps of zero */
    buffer[0].timestamp = -1;
    buffer[0].message = Pm_Message((unsigned char)(144+channelid), (unsigned char)notenumber, (unsigned char)velocity); //for note on, status=144+channelid
	buffer[1].timestamp = -1; 
	buffer[1].message = Pm_Message((unsigned char)(144+channelid), (unsigned char)(notenumber), (unsigned char)0); //for note off, one way of doing it is with, same status=144+channelid, same note number but with velocity 0
	//buffer[1].message = Pm_Message((unsigned char)(128+channelid), (unsigned char)(notenumber), (unsigned char)0); //for note off, another way of doing it is with, status=128+channelid, same note number and any velocity

	if(duration !=-1 && duration!=0)
	{
		Pm_WriteShort(midi, buffer[0].timestamp, buffer[0].message);
		Sleep(duration+latency);
		Pm_WriteShort(midi, buffer[1].timestamp, buffer[1].message);

		if(close_mididevice_onexit==TRUE)
		{
			//close device (this not explicitly needed in most implementations) 
			Pm_Close(midi);
			/*
			Pm_Terminate();
			*/
			#ifdef DEBUG
				printf("done closing and terminating...\n");
			#endif //DEBUG
		}
	}
	else
	{
		Pm_WriteShort(midi, buffer[0].timestamp, buffer[0].message);
	}

}


void midinote(const char* mididevicename, int channelid, int notenumber, int duration, int velocity) 
{
	int latency = global_latency; //should be 0 or close to 0
	int deviceid = get_device_id(mididevicename, STRING_MAX);
	const PmDeviceInfo* info;
	char devicename[STRING_MAX];
	if(deviceid==-1) 
	{
		deviceid=Pm_GetDefaultOutputDeviceID();
		//printf("warning wrong devicename syntax, replacing invalid devicename by default devicename\n");
		info = Pm_GetDeviceInfo(deviceid);
		strcpy(devicename, info->name);
		//printf("device name %s\n", devicename);
	}
	int close_mididevice_onexit = false;

	midinote_nostream(latency, deviceid, close_mididevice_onexit, channelid, notenumber, duration, velocity); 
}


//the purpose of creating nostream() version was to attempt to prevent cc 64 values being sent on all channels when closing mididevice
//by relying on Pm_WriteShort to send midi message instead of Pm_Write to send midi event (a midi message with timestamp) but did not
//find a way to send midi message without timestamp other than by specifying zero latency when opening the mididevice which provoques
//timestamps to be ignored later on when sending mesage onto this device and thereby prevents cc 64 value 0 to be sent on all channels
//upon closing mididevice.
void midiprogramchange_nostream(int latency, int deviceid, int close_mididevice_onexit, int channelid, int programnumber) 
{
    PmStream* midi;
	char line[80];
    PmEvent buffer[3];

	/*
    //It is recommended to start timer before PortMidi 
    TIME_START;
	*/

	/* open output device */
    Pm_OpenOutput(&midi, 
                  deviceid, 
                  DRIVER_INFO,
                  OUTPUT_BUFFER_SIZE, 
                  TIME_PROC,
                  TIME_INFO, 
                  latency);
#ifdef DEBUG
    printf("Midi Output opened with %ld ms latency.\n", (long) latency);
#endif //DEBUG

    /* writing midi for immediate output, we use timestamps of zero */
    buffer[0].timestamp = -1;
    buffer[0].message = Pm_Message((unsigned char)(192+channelid), (unsigned char)programnumber, (unsigned char)0);

	Pm_WriteShort(midi, buffer[0].timestamp, buffer[0].message);

	if(close_mididevice_onexit==TRUE)
	{
		//close device (this not explicitly needed in most implementations) 
		Pm_Close(midi);
		/*
		Pm_Terminate();
		*/
		#ifdef DEBUG
			printf("done closing and terminating...\n");
		#endif //DEBUG
	}

}


void midiprogramchange(const char* mididevicename, int channelid, int programnumber) 
{
	int latency = global_latency; //should be 0 or close to 0
	int deviceid = get_device_id(mididevicename, STRING_MAX);
	const PmDeviceInfo* info;
	char devicename[STRING_MAX];
	if(deviceid==-1) 
	{
		deviceid=Pm_GetDefaultOutputDeviceID();
		//printf("warning wrong devicename syntax, replacing invalid devicename by default devicename\n");
		info = Pm_GetDeviceInfo(deviceid);
		strcpy(devicename, info->name);
		//printf("device name %s\n", devicename);
	}
	int close_mididevice_onexit = false;

	midiprogramchange_nostream(latency, deviceid, close_mididevice_onexit, channelid, programnumber); 
}


//caller passes input parameters
//caller allocates output parameters (for this function to compute):
//double fNumEvents = 0; //number of additional events, after first is sent on startdelay
//double fEventDelay = 0;
void compute_number_of_events_and_relative_delay(int ccstart, int ccend, int ccstep, int serieduration_ms, int* pnNumEvents, double* pfEventDelay)
{
    int nNumEvents = *pnNumEvents; //number of additional events, after first is sent on startdelay
    double fEventDelay = *pfEventDelay;
    double fNumEvents = 0; //number of additional events, after first is sent on startdelay
    if(ccstep!=0)
    {
        fNumEvents = abs((ccend - ccstart)/ccstep);
        nNumEvents = (int) ceil(fNumEvents);
        fEventDelay = (serieduration_ms+0.0f)/(nNumEvents+0.0f); 
    }
    else
    {
        //ccstep==0
        if(ccend!=ccstart)
        {
            fNumEvents=1;
            nNumEvents=(int)fNumEvents;
            fEventDelay = serieduration_ms;
        }
        else
        {
            //fNumEvents=0;
            fNumEvents=1; //spi 2014
            nNumEvents=(int)fNumEvents;
            fEventDelay=0;
        }
    }            
	#ifdef DEBUG 
		printf("fNumEvents: %f\n", fNumEvents);
		printf("nNumEvents: %d additional events (aside first event)\n", nNumEvents);
		printf("fEventDelay: %f ms\n", fEventDelay);
	#endif
    *pnNumEvents = nNumEvents; //write to output parameter
    *pfEventDelay = fEventDelay; //write to output parameter
	return;
}


//the purpose of creating nostream() version was to attempt to prevent cc 64 values being sent on all channels when closing mididevice
//by relying on Pm_WriteShort to send midi message instead of Pm_Write to send midi event (a midi message with timestamp) but did not
//find a way to send midi message without timestamp other than by specifying zero latency when opening the mididevice which provoques
//timestamps to be ignored later on when sending mesage onto this device and thereby prevents cc 64 value 0 to be sent on all channels
//upon closing mididevice.
void midicontinuouscontroller_nostream(int latency, int deviceid, int close_mididevice_onexit, int channelid, int ccnumber, int ccstart, int ccend, int ccstep, int startdelay_ms, int serieduration_ms) 
{
	int ccvalue = 0;
	int index=0; 
	int ccnext=0;
	int nNumEvents = 0; 
	double fShouldBeDelay = 0;
	long iRelativeDelay = 0;
	double fEventDelay = 0;
    PmStream* midi;
	char line[80];
    PmEvent buffer[3];

	/*
    //It is recommended to start timer before PortMidi 
    TIME_START;
	*/

	/* open output device */
    Pm_OpenOutput(&midi, 
                  deviceid, 
                  DRIVER_INFO,
                  OUTPUT_BUFFER_SIZE, 
                  TIME_PROC,
                  TIME_INFO, 
                  latency);
#ifdef DEBUG
    printf("Midi Output opened with %ld ms latency.\n", (long) latency);
#endif //DEBUG

	//int nNumEvents = 0; 
	//double fEventDelay = 0;
	compute_number_of_events_and_relative_delay(ccstart, ccend, ccstep, serieduration_ms, &nNumEvents, &fEventDelay);


    ////////////////////////////////////////////////////////
    //send all events delayed by proper thread sleeping time 
    ////////////////////////////////////////////////////////
    //int index=0;
    //int nCC_next=0;
    for(index=0; index<(nNumEvents); index++)
    { 
        ccnext = index*ccstep + ccstart; 
        //fShouldBeDelay = fEventDelay * (index+0.0f); // + nStartDelay; 
        fShouldBeDelay = fEventDelay; //spi 2014
        iRelativeDelay = (long)(fShouldBeDelay+0.5); //round(fShouldBeDelay);  
        if(index==0) {Sleep((DWORD)startdelay_ms);}
            else {Sleep((DWORD)iRelativeDelay);}
		// writing midi for immediate output, we use timestamps of zero
		buffer[0].timestamp = -1;
		buffer[0].message = Pm_Message((unsigned char)(176+channelid), (unsigned char)ccnumber, (unsigned char)ccnext);
		Pm_WriteShort(midi, buffer[0].timestamp, buffer[0].message);
    } 
    //check to see if we still need to send ccEnd value             
    if( (ccstep>0)&&(ccnext<ccend) || (ccstep<0)&&(ccnext>ccend) ) //if (nCC_next < nCC_end) 
    { 
        //fShouldBeDelay = fEventDelay * (index+0.0f);// + nStartDelay; 
        fShouldBeDelay = fEventDelay; //spi 2014
        iRelativeDelay = (long)(fShouldBeDelay+0.5); //round(fShouldBeDelay);  
        Sleep((DWORD)iRelativeDelay);
        ccnext = ccend;
		//writing midi for immediate output, we use timestamps of zero
		buffer[0].timestamp = -1;
		buffer[0].message = Pm_Message((unsigned char)(176+channelid), (unsigned char)ccnumber, (unsigned char)ccnext);
		Pm_WriteShort(midi, buffer[0].timestamp, buffer[0].message);
    } 

	if(close_mididevice_onexit==TRUE)
	{
		/* close device (this not explicitly needed in most implementations) */
		Pm_Close(midi);
		/*
		Pm_Terminate();
		*/
		#ifdef DEBUG
			printf("done closing and terminating...\n");
		#endif //DEBUG
	}

}


void midicontinuouscontroller(const char* mididevicename, int channelid, int ccnumber, int ccstart, int ccend, int ccstep, int startdelay_ms, int serieduration_ms) 
{
	int latency = global_latency; //should be 0 or close to 0
	int deviceid = get_device_id(mididevicename, STRING_MAX);
	const PmDeviceInfo* info;
	char devicename[STRING_MAX];
	if(deviceid==-1) 
	{
		deviceid=Pm_GetDefaultOutputDeviceID();
		//printf("warning wrong devicename syntax, replacing invalid devicename by default devicename\n");
		info = Pm_GetDeviceInfo(deviceid);
		strcpy(devicename, info->name);
		//printf("device name %s\n", devicename);
	}
	int close_mididevice_onexit = false;

	midicontinuouscontroller_nostream(latency, deviceid, close_mididevice_onexit, channelid, ccnumber, ccstart, ccend, ccstep, startdelay_ms, serieduration_ms);
}