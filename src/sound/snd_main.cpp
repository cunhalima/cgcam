#include "../defs.h"
#if 0

#include <cmath>
#include <portaudio.h>

#define SAMPLE_RATE				44100
#define FRAMES_PER_BUFFER		4096

#ifndef M_PI
#define M_PI  (3.14159265)
#endif

#define TABLE_SIZE   (200)

//lovk free http://www.portaudio.com/docs/v19-doxydocs/paex__ocean__shore_8c_source.html
struct snddata_t {
    float sine[TABLE_SIZE];
    int left_phase;
    int right_phase;
    char message[20];
};
static snddata_t data;
static PaStream *stream;
static bool snd_on = false;

static void StreamFinished(void *userData) {
	snddata_t *data = (snddata_t *)userData;
	//printf( "Stream Completed: %s\n", data->message );
}

// This routine will be called by the PortAudio engine when audio is needed.
// It may called at interrupt level on some machines so don't do anything
// that could mess up the system like calling malloc() or free().
//
static int paCallback(const void *inputBuffer, void *outputBuffer,
                          unsigned long framesPerBuffer,
                          const PaStreamCallbackTimeInfo* timeInfo,
                          PaStreamCallbackFlags statusFlags,
                          void *userData)
{
    snddata_t *data = (snddata_t*)userData;
    float *out = (float*)outputBuffer;
    unsigned long i;
   
    for (i = 0; i < framesPerBuffer; i++) {
        *out++ = data->sine[data->left_phase];  /* left */
        *out++ = data->sine[data->right_phase];  /* right */
        data->left_phase += 1;
        if( data->left_phase >= TABLE_SIZE ) data->left_phase -= TABLE_SIZE;
        data->right_phase += 3; /* higher pitch so we can distinguish left and right. */
        if( data->right_phase >= TABLE_SIZE ) data->right_phase -= TABLE_SIZE;
    }
    return paContinue;
}

bool SND_Init() {
    PaStreamParameters outputParameters;
    PaError err;

    for(int i=0; i<TABLE_SIZE; i++ )
    {
        data.sine[i] = (float) sin( ((double)i/(double)TABLE_SIZE) * M_PI * 2. );
    }
    data.left_phase = data.right_phase = 0;

	err = Pa_Initialize();
    if (err != paNoError) {
		return false;
	}
    outputParameters.device = Pa_GetDefaultOutputDevice();
    if (outputParameters.device == paNoDevice) {
		//fprintf(stderr,"Error: No default output device.\n");
	    Pa_Terminate();
		return false;
    }
    outputParameters.channelCount = 2; // stereo
    outputParameters.sampleFormat = paFloat32; // 32 bit floating point output
    outputParameters.suggestedLatency = Pa_GetDeviceInfo(outputParameters.device)->defaultLowOutputLatency;
    outputParameters.hostApiSpecificStreamInfo = NULL;
    err = Pa_OpenStream(
              &stream,
              NULL, /* no input */
              &outputParameters,
              SAMPLE_RATE,
              FRAMES_PER_BUFFER,
              paClipOff,      /* we won't output out of range samples so don't bother clipping them */
              paCallback,
              &data);
    if (err != paNoError) {
	    Pa_Terminate();
		return false;
	}
    err = Pa_SetStreamFinishedCallback(stream, &StreamFinished);
    if (err != paNoError) {
		Pa_CloseStream(stream);
	    Pa_Terminate();
		return false;
	}
    err = Pa_StartStream(stream);
    if (err != paNoError) {
		Pa_CloseStream(stream);
	    Pa_Terminate();
		return false;
	}
	return true;
}

void SND_Cleanup() {
	if (!snd_on) {
		return;
	}
    PaError err;
    err = Pa_StopStream(stream);
    if (err == paNoError) {
	    Pa_CloseStream(stream);
	}
    Pa_Terminate();
	snd_on = false;
}

#else

bool SND_Init() {
    return true;
}

void SND_Cleanup() {
}

#endif
