#include <finite/audio.h>

int main() {
    FinitePlaybackDevice *dev = finite_audio_device_init();
    char *jingle = "jingle.mp3";

    finite_audio_get_audio_params(jingle, dev);
    // print out the audio duration
    finite_audio_get_audio_duration(dev);

    // use params to init audio
    finite_audio_init_audio(dev, jingle, false);

    // audio is made so now play

    finite_audio_play(dev);
    printf("Done\n");
    // clean up when finished
    finite_audio_cleanup(dev);
}