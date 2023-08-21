#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "portaudio.h"

#define SAMPLE_RATE 44100
#define FRAMES_PER_BUFFER 512

static void handle_err(PaError err) {
	if(err != paNoError) {
		printf("ERR: %s\n", Pa_GetErrorText(err));
		exit(EXIT_FAILURE);
	}
}

void print_device_info(int device) 
{
	const PaDeviceInfo* device_info;
	device_info = Pa_GetDeviceInfo(device);
	printf("Device info of %d\n:", device);
	printf("	Name: %s\n", device_info->name);
	printf("	MaxInputChannels: %d\n", device_info->maxInputChannels);
	printf("	MaxOutputChannels: %d\n", device_info->maxOutputChannels);
	printf("	MaxOutputChannels: %f\n", device_info->defaultSampleRate);
}

static int stream_callback(const void* ibuf, void* obuf, unsigned long framesPerBuffer, const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags, void* userData) 
{
	(void)obuf;
	float* in = (float*)ibuf;

	float max = 0;

	for(int i=0; i<FRAMES_PER_BUFFER; i++) {
		float v = in[i];
		if(v < 0) {
			v = -v;
		}

		if(in[i] > max) {
			max = in[i];
		}
	}
	printf("%f\n", max);
	return 0;
}

int main(void)
{
	PaError err;

	err = Pa_Initialize();
	handle_err(err);
	
	int device_count = Pa_GetDeviceCount();
	if(device_count < 0) {
		printf("Error getting device count.\n");
		exit(EXIT_FAILURE);
	} else if(device_count == 0) {
		printf("Not enough devices!\n");
		exit(EXIT_FAILURE);
	}
	
	for(int i=0; i<device_count; ++i) {
		print_device_info(i);
	}

	int input_device = 0;
	int output_device = 1;

	PaStreamParameters ipams;
	PaStreamParameters opams;

	memset(&ipams, 0, sizeof(ipams));

	ipams.channelCount = 1;
	ipams.device = input_device;
	ipams.hostApiSpecificStreamInfo = NULL;
	ipams.sampleFormat = paFloat32;
	ipams.suggestedLatency = Pa_GetDeviceInfo(input_device)->defaultLowInputLatency;
	
	memset(&opams, 0, sizeof(ipams));

	opams.channelCount = 2;
	opams.device = output_device;
	opams.hostApiSpecificStreamInfo = NULL;
	opams.sampleFormat = paFloat32;
	opams.suggestedLatency = Pa_GetDeviceInfo(output_device)->defaultLowInputLatency;

	PaStream* stream;

	err = Pa_OpenStream(
		&stream,
		&ipams,
		&opams,
		SAMPLE_RATE,
		FRAMES_PER_BUFFER,
		paNoFlag,
		&stream_callback,
		NULL
	);
	handle_err(err);

	err = Pa_StartStream(stream);
	handle_err(err);

	Pa_Sleep(100 * 1000);

	err = Pa_StopStream(stream);
	handle_err(err);

	err = Pa_CloseStream(stream);
	handle_err(err);

	err = Pa_Terminate();
	handle_err(err);

	return EXIT_SUCCESS;
}
