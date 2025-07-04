#include "../include/audio/audio.h"

snd_stream_t getAudioModeFromFinite(FiniteAudioMode mode) {
    switch (mode) {
        case FINITE_AUDIO_MODE_PLAYBACK:
            return SND_PCM_STREAM_PLAYBACK;
    }
}

FinitePlaybackDevice *finite_audio_device_init(char *name) {
    FinitePlaybackDevice *dev = calloc(1, sizeof(FinitePlaybackDevice));

    // if name is NULL use default
    if (!name) {
        name = "default";
    }

    int err = snd_pcm_open(&dev->device, name, SND_PCM_STREAM_PLAYBACK, 0);
    if (err != 0) {
        printf("Unable to open playback device %d\n", err);
    }

    dev->name = name;

    return dev;
}

void *finite_audio_device_hw(FinitePlaybackDevice *dev, bool isOnHeap) {
    if (!dev)
}