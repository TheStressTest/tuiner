#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "portaudio.h"
#include "fftw3.h"
#include <complex.h>

#define SAMPLE_RATE 20000
#define NOISE_CUTOFF 10
#define FRAMES_PER_BUFFER 8192
#define FFT_SIZE 8192

#define FFT_INDEX_TO_FREQ(i) ((float)i * SAMPLE_RATE / FFT_SIZE)

fftwf_complex* fft_result;
fftwf_plan fft_plan;

// These are the frequencies after using HSP (Harmonic product spectrum)
float* processed;
// These are the magnitudes after applying a noise filter.
float* magnitudes;

static double peak_freq = -1.0;

char* midi_to_note(int midi_note)
{
    char* note_letters[] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};

    int note_index = midi_note % 12;
    return note_letters[note_index];
}

int midi_to_octave(int midi_note)
{
    return ((midi_note / 12) - 1);
}

static void handle_err(PaError err)
{
	if(err != paNoError) {
		printf("ERR: %s\n", Pa_GetErrorText(err));
		exit(EXIT_FAILURE);
	}
}

static void get_magnitudes()
{
    int i, max_i = -1;

	for (i=0; i<FFT_SIZE-1; i++) {
		/* out[0] is a DC result */
        _Complex float complex_value = *(_Complex float*)(&fft_result[i+1]);
        // Absolute value all results.
		magnitudes[i] = cabsf(complex_value);

		if (magnitudes[i] < NOISE_CUTOFF) {
			magnitudes[i] = 0.0;
		}

		if (magnitudes[i] > magnitudes[max_i]) {
			max_i = i;
		}
	}
	peak_freq = FFT_INDEX_TO_FREQ(max_i);
}

static void compute_hps()
{
	int i;

	for (i=0; i<FFT_SIZE; i++) {
		int j;
		processed[i] = magnitudes[i];
		/* next 4 harmonics */
		for (j=2; j<=5; j++) {
			if (i*j >= FFT_SIZE) {
				break;
			}
			if (magnitudes[i*j] < 0.00001) {
				/* too close to zero */
				continue;
			}
			processed[i] *= magnitudes[i*j];
		}
	}
}

static int stream_callback(const void* ibuf, void* obuf, unsigned long framesPerBuffer, const PaStreamCallbackTimeInfo* timeInfo, PaStreamCallbackFlags statusFlags, void* userData)
{
	(void)obuf;
	float* in = (float*)ibuf;

    float max = 0;

    // Quick way to calculate max amplitude.
    for(int i=0; i<FRAMES_PER_BUFFER; i++) {
		float v = in[i];
		if(v < 0) {
			v = -v;
		}

		if(in[i] > max) {
			max = in[i];
		}
	}

    // Don't care too much about random noise.
    if (max < 0.02)
        return 0;



    fft_result = fftwf_alloc_complex(FFT_SIZE);
    processed = fftwf_alloc_real(FFT_SIZE);
    magnitudes = fftwf_alloc_real(FFT_SIZE);
    fft_plan = fftwf_plan_dft_r2c_1d(FRAMES_PER_BUFFER, in, fft_result, FFTW_ESTIMATE);

    fftwf_execute(fft_plan);

    get_magnitudes();
    compute_hps();
    int max_i = -1;
    for (int i=0; i<FFT_SIZE; i++) {
		if (processed[i] > processed[max_i]) {
			max_i = i;
		}
	}
    float dominant_frequency = FFT_INDEX_TO_FREQ(max_i);
    if (dominant_frequency < 0) {
        return 0;
    }

    // This is the formula to convert a frequency in Hz to a midi note number.
    int note_number = round(12 * log2(dominant_frequency / 440) + 69);

    char* note = midi_to_note(note_number);
    int octave = midi_to_octave(note_number);
    printf("\033[2J");

    char* logo = "  ______      _                \n"
                 " /_  __/_  __(_)___  ___  _____\n"
                 "  / / / / / / / __ \\/ _ \\/ ___/\n"
                 " / / / /_/ / / / / /  __/ /    \n"
                 "/_/  \\__,_/_/_/ /_/\\___/_/     \n";

    printf("%s\n", logo);
    printf("Frequency: ~%.2f\n", dominant_frequency);
    printf("%d\t%d\t%d\t%d\t%d\t\e[1;32m%d\e[0m\t%d\t%d\t%d\t%d\t%d\n",
            midi_to_octave(note_number - 5),
            midi_to_octave(note_number - 4),
            midi_to_octave(note_number - 3),
            midi_to_octave(note_number - 2),
            midi_to_octave(note_number - 1),
            midi_to_octave(note_number),
            midi_to_octave(note_number + 1),
            midi_to_octave(note_number + 2),
            midi_to_octave(note_number + 3),
            midi_to_octave(note_number + 4),
            midi_to_octave(note_number + 5)
    );

    printf("%s\t%s\t%s\t%s\t%s\t\e[1;32m%s\e[0m\t%s\t%s\t%s\t%s\t%s\n",
            midi_to_note(note_number - 5),
            midi_to_note(note_number - 4),
            midi_to_note(note_number - 3),
            midi_to_note(note_number - 2),
            midi_to_note(note_number - 1),
            midi_to_note(note_number),
            midi_to_note(note_number + 1),
            midi_to_note(note_number + 2),
            midi_to_note(note_number + 3),
            midi_to_note(note_number + 4),
            midi_to_note(note_number + 5)
    );
    printf("\t\t\t\t\t^\n");
    printf("(Press enter to quit.)\n");
    fftwf_destroy_plan(fft_plan);
    fftwf_free(fft_result);
    fftwf_free(processed);
    fftwf_free(magnitudes);

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

    // In the future there can be a command line option to display device information and even choose a non default device.
/* 	for(int i=0; i<device_count; ++i) { */
/* 		get_device_info(i); */
/* 	} */

    int input_device = Pa_GetDefaultInputDevice();
    const PaDeviceInfo* input_info = Pa_GetDeviceInfo(input_device);
    int output_device = Pa_GetDefaultOutputDevice();
    printf("Input: %s\n(Play note to see scale.)\n", input_info->name);

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

    // Wait for enter key
	getchar();

	err = Pa_StopStream(stream);
	handle_err(err);

	err = Pa_CloseStream(stream);
	handle_err(err);

	err = Pa_Terminate();
	handle_err(err);

	return EXIT_SUCCESS;
}
