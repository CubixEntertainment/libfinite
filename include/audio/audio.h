#ifndef __AUDIO_H_
#define __AUDIO_H_
#include <alsa/asoundlib.h>

// refers to the device
typedef struct FinitePlaybackDevice FinitePlaybackDevice;

enum FiniteAudioMode {
    FINITE_AUDIO_MODE_PLAYBACK
}

struct FinitePlaybackDevice {
    char *name;
    snd_pcm_t *device;
    snd_pcm_format_t format;
    snd_pcm_hw_params_t *params;
    uint32_t sample_rate;
    uint32_t channel;
    uint32_t buff_time;
    uint32_t buff_per;
    double freq;
    int verbose;
    int resample;
    int per_event;
};

FinitePlaybackDevice *finite_audio_device_init(char *name);

#endif