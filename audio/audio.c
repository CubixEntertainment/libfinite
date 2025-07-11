#include "../include/audio/audio.h"

void finite_audio_get_audio_duration(FinitePlaybackDevice *dev) {
    // to get true seconds do basic math

    if (dev->sfFrames <= 0 || dev->sample_rate <= 0) {
        printf("Unable to calculate duration on incomplete file. \n");
        exit(EXIT_FAILURE);
    }

    double time = dev->sfFrames / dev->sample_rate;
    printf("true time is %f\n", time);

    int hours = (int) time / 3600;
    int minutes = ((int) time / 60) - (60 * hours);
    int seconds = ((int) time) - ((minutes * 60) + (hours * 3600));
    int ms = (time - (int) time) * 1000;  

    printf("Time: %d:%d:%d:%d\n", hours, minutes, seconds,ms);

    dev->dur.hours = hours;
    dev->dur.minutes = minutes;
    dev->dur.seconds = seconds;
    dev->dur.milliseconds = ms;
}

bool finite_audio_get_audio_params(char *file, FinitePlaybackDevice *dev) {
    // verify the file exists
    if (access(file, F_OK) != 0) {
        printf("Unable to open file at %s\n", file);
        exit(EXIT_FAILURE);
    }

    if (!dev) {
        printf("Unable to assign data to NULL device\n");
    }

    SF_INFO info;

    dev->file = sf_open(file, SFM_READ, &info);
    if (!dev->file) {
        printf("Something went wrong \n");
        return false;
    }

    // now assign the info data to the device
    printf("Sound supports %d channels. %s\n", info.channels, info.channels == 2 ? "(Stereo)" : "(Mono)");
    dev->channels = info.channels;
    if (info.samplerate != 44100) {
        printf("Sample Rate %d is not the recommended sample rate of 44100.\n", info.samplerate);
    }

    dev->sample_rate = (uint32_t) info.samplerate;
    
    dev->sfFrames = info.frames;
    return true;
}

FinitePlaybackDevice *finite_audio_device_init() {
    FinitePlaybackDevice *dev = calloc(1, sizeof(FinitePlaybackDevice));

    int err = snd_pcm_open(&dev->device, "default", SND_PCM_STREAM_PLAYBACK, 0);
    if (err < 0) {
        printf("Unable to open device\n");
        return 0;
    }

    dev->name = "default";

    err = snd_pcm_hw_params_malloc(&dev->params);
    if (err < 0) {
        printf("Unable to allocate hw params\n");
        return 0;
    }

    return dev;
};

// when autoCreate is true it will run finite_audio_get_audio_params(). 
bool finite_audio_init_audio(FinitePlaybackDevice *dev, char* audio, bool autoCreate) {
    if (!dev || !audio) {
        printf("Unable to init audio with NULL data");
        return false;
    } 

    if (autoCreate) {
        bool success = finite_audio_get_audio_params(audio, dev);
        if (!success) {
            return false; // message is printed already
        }
    }

    snd_pcm_hw_params_any(dev->device, dev->params);

    int err;

    err = snd_pcm_hw_params_set_rate_resample(dev->device, dev->params, dev->resample);
    if (err < 0) {
        printf("Unable to set resample rate\n");
        return false;
    }

    err = snd_pcm_hw_params_set_access(dev->device, dev->params, SND_PCM_ACCESS_RW_INTERLEAVED);
    if (err < 0) {
        printf("Unable to set device sampling to interleaved.\n");
        return false;
    }

    err = snd_pcm_hw_params_set_format(dev->device, dev->params, SND_PCM_FORMAT_S16_LE);
    if (err < 0) {
        printf("Unable to set device format.\n");
        return false;
    }

    dev->format = SND_PCM_FORMAT_S16_LE;

    err = snd_pcm_hw_params_set_channels(dev->device, dev->params, dev->channels);

    if (err < 0) {
        printf("Unable to set device channels.\n");
        return false;
    }

    err = snd_pcm_hw_params_set_rate(dev->device, dev->params, dev->sample_rate, 0);
    if (err < 0) {
        printf("Unable to set playback rate\n");
        return false;
    }

    err = snd_pcm_hw_params(dev->device, dev->params);
    if (err < 0) {
        printf("Unable to save params to device\n");
        return false;
    }

    err = snd_pcm_prepare(dev->device);
    if (err < 0) {
        printf("Unable to prepare the device\n");
        return false;
    }

    dev->filename = audio;
    return true;
}

void finite_audio_play(FinitePlaybackDevice *dev) {
    if (!dev) {
        printf("Unable to init audio with NULL device");
    } 

    snd_pcm_hw_params_get_buffer_size(dev->params, &dev->frames);
    printf("Got audio device with size %ld\n", dev->frames);

    snd_pcm_hw_params_get_period_size(dev->params, &dev->frames, 0);
    size_t buf_size = dev->frames * dev->channels * sizeof(short);
    dev->audioBuffer = malloc(buf_size);

    // clean up params
    snd_pcm_hw_params_free(dev->params);

    short _read;
    sf_count_t _frames = dev->frames;
    dev->isPlaying = true;
    dev->isPaused = false;
    while (true) {
        snd_pcm_state_t state = snd_pcm_state(dev->device);
        if (state != SND_PCM_STATE_PAUSED) {
            if ((_read = sf_readf_short(dev->file, dev->audioBuffer, _frames)) == 0) {
                break;
            }

            int pcm_data = snd_pcm_writei(dev->device, dev->audioBuffer, _read);
            if (pcm_data == -EPIPE) {
                printf("An underrun occurred.\n");
                snd_pcm_prepare(dev->device);
            } else if (pcm_data < 0) {
                printf("Unable to write to device %s\n", snd_strerror(pcm_data));
            } else if (pcm_data != _read) {
                printf("Write data does not match read data.\n");
            }
        }
    }

    printf("Read succesfully. Audio file %s has %d channel(s) with a sample rate of %d\n", dev->filename, dev->channels, dev->sample_rate);
    snd_pcm_drain(dev->device);
}

bool finite_audio_stop(FinitePlaybackDevice *dev) {
    if (!dev) {
        printf("Unable to stop audio with NULL device");
        return false;
    } 

    if (!dev->isPlaying) {
        printf("Unable to stop audio that isn't playing.");
        return false;
    }

    snd_pcm_drop(dev->device);
    return true;
}

bool finite_audio_pause(FinitePlaybackDevice *dev) {
    if (!dev) {
        printf("Unable to stop audio with NULL device\n");
        return false;
    } 

    if (dev->isPlaying == false) {
        printf("Unable to toggle pause on an audio that isn't initialized.\n");
        return false;
    }

    if (dev->isPaused == true) {
        snd_pcm_pause(dev->device, 0);
        dev->isPaused = false;
    } else {
        snd_pcm_pause(dev->device, 1);
        dev->isPaused = true;
    }
    return true;
}

bool finite_audio_unpause(FinitePlaybackDevice *dev) {
    if (!dev) {
        printf("Unable to stop audio with NULL device");
        return false;
    } 

    if (!dev->isPlaying || !dev->isPaused) {
        printf("Unable to unpause audio that isn't pauses.");
        return false;
    }

    snd_pcm_pause(dev->device, 0);
    return true;
}

void finite_audio_cleanup(FinitePlaybackDevice *dev) {
    if (!dev) {
        printf("Unable to cleanup NULL device");
    } else {
        if (dev->audioBuffer) {
            snd_pcm_close(dev->device);
            free(dev->audioBuffer);
        }
        if (dev->file) {
            sf_close(dev->file);
        }
        free(dev);
    }
}