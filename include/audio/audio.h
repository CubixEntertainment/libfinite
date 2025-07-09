#ifndef __AUDIO_H_
#define __AUDIO_H_
#include <alsa/asoundlib.h>
#include <sndfile.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>

// refers to the device
typedef struct FinitePlaybackDevice FinitePlaybackDevice;
typedef struct FinitePlaybackDuration FinitePlaybackDuration;
struct FinitePlaybackDuration {
    double trueSeconds;
    int hours;
    int minutes;
    int seconds;
    int milliseconds;
};

struct FinitePlaybackDevice {
    char *name;
    char *filename;
    bool isPlaying;
    bool isPaused;
    short *audioBuffer;
    FinitePlaybackDuration dur;
    snd_pcm_t *device;
    snd_pcm_format_t format;
    snd_pcm_hw_params_t *params;
    snd_pcm_uframes_t frames;
    SNDFILE *file;
    sf_count_t sfFrames;
    uint32_t sample_rate;
    uint32_t channels;
    uint32_t buff_time;
    uint32_t buff_per;
    double freq;
    int verbose;
    int resample;
    int per_event;
};
void finite_audio_get_audio_duration(FinitePlaybackDevice *dev);
bool finite_audio_get_audio_params(char *file, FinitePlaybackDevice *dev);
FinitePlaybackDevice *finite_audio_device_init();
bool finite_audio_init_audio(FinitePlaybackDevice *dev, char* audio, bool autoCreate);
void finite_audio_play(FinitePlaybackDevice *dev);
bool finite_audio_stop(FinitePlaybackDevice *dev);
bool finite_audio_pause(FinitePlaybackDevice *dev);
bool finite_audio_unpause(FinitePlaybackDevice *dev);
void finite_audio_cleanup(FinitePlaybackDevice *dev);

#endif