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

#define finite_audio_get_audio_duration(dev) finite_audio_get_audio_duration_debug(__FILE__, __func__, __LINE__, dev)
void finite_audio_get_audio_duration_debug(const char *file, const char *func, int line, FinitePlaybackDevice *dev);

#define finite_audio_get_audio_params(file, dev) finite_audio_get_audio_params_debug(__FILE__, __func__, __LINE__, file, dev)
bool finite_audio_get_audio_params_debug(const char *rfile, const char *func, int line, char *file, FinitePlaybackDevice *dev);

#define finite_audio_device_init() finite_audio_device_init_debug(__FILE__, __func__, __LINE__)
FinitePlaybackDevice *finite_audio_device_init_debug(const char *file, const char *func, int line);

#define finite_audio_init_audio(dev, audio, autoCreate) finite_audio_init_audio_debug(const char *file, const char *func, int line, dev, audio, autoCreate)
bool finite_audio_init_audio_debug(const char *file, const char *func, int line, FinitePlaybackDevice *dev, char* audio, bool autoCreate);

#define finite_audio_play(dev) finite_audio_play_debug(__FILE__, __func__, __LINE__, dev)
void finite_audio_play_debug(const char *file, const char *func, int line, FinitePlaybackDevice *dev);

#define finite_audio_stop(dev) finite_audio_stop_debug(__FILE__, __func__, __LINE__, dev)
bool finite_audio_stop_debug(const char *file, const char *func, int line, FinitePlaybackDevice *dev);

#define finite_audio_pause(dev) finite_audio_pause_debug(__FILE__, __func__, __LINE__, dev)
bool finite_audio_pause_debug(const char *file, const char *func, int line, FinitePlaybackDevice *dev);

#define finite_audio_unpause(dev) finite_audio_unpause_debug(__FILE__, __func__, __LINE__, dev)
bool finite_audio_unpause_debug(const char *file, const char *func, int line, FinitePlaybackDevice *dev);

#define finite_audio_cleanup(dev) finite_audio_cleanup_debug(__FILE__, __func__, __LINE__, dev)
void finite_audio_cleanup_debug(const char *file, const char *func, int line, FinitePlaybackDevice *dev);

#endif