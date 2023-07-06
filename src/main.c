#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <stdbool.h>
#include <time.h>

#include <argp.h>

#include <rtaudio/rtaudio_c.h>
#include "chimes.h"

static int
audio_cb(void *outputBuffer, void *inputBuffer, unsigned int nBufferFrames,
         double stream_time, rtaudio_stream_status_t status, void *data)
{
    chime_t *chime = data;
    float *buffer = (float *)outputBuffer;

    for (unsigned int i = 0; i < nBufferFrames; i++)
    {
        buffer[i] = chime_update(chime);
    }

    if (chime_is_finished(chime))
    {
        return 1;
    }
    else
    {

        return 0;
    }
}

static void error_cb(rtaudio_error_t err, const char *msg)
{
    fprintf(stderr, "Error type: %d message: %s\n", err, msg);
}

int main(int argc, char *argv[])
{
    srand(time(NULL));
    unsigned int bufferFrames = SAMPLERATE * 1;
    const rtaudio_api_t api = rtaudio_compiled_api_by_name("pulse");
    fprintf(stderr, "RTAudio API: %s\n", rtaudio_api_name(api));

    rtaudio_t dac = rtaudio_create(api);

    int n = rtaudio_device_count(dac);

    if (n == 0)
    {
        fprintf(stderr, "No audio devices available\n");
        exit(EXIT_FAILURE);
    }
    else if (n == 1)
    {
        fprintf(stderr, "There is %d audio device available\n", n);
    }
    else
    {
        fprintf(stderr, "There are %d audio devices available:\n", n);
    }

    int dev = rtaudio_get_default_output_device(dac);

    for (int i = 0; i < n; i++)
    {
        rtaudio_device_info_t info = rtaudio_get_device_info(dac, i);
        if (info.probed)
        {
            fprintf(stderr, "\t\"%s\"%s\n", info.name, dev == i ? " (default)" : "");
        }
        else
        {
            fprintf(stderr, "Failed to read info from audio device: %d\n", i);
        }
    }

    rtaudio_show_warnings(dac, true);

    rtaudio_stream_parameters_t o_params = {
        .device_id = rtaudio_get_default_output_device(dac),
        .first_channel = 0,
        .num_channels = 1};

    rtaudio_stream_options_t options = {
        .flags = RTAUDIO_FLAGS_HOG_DEVICE | RTAUDIO_FLAGS_MINIMIZE_LATENCY | RTAUDIO_FLAGS_NONINTERLEAVED};

    chime_t *chime = chime_init();
    assert(chime);

    rtaudio_error_t err = rtaudio_open_stream(dac, &o_params, NULL,
                                              RTAUDIO_FORMAT_FLOAT32,
                                              SAMPLERATE, &bufferFrames, &audio_cb,
                                              (void *)chime, &options,
                                              &error_cb);
    assert(err == 0);

    err = rtaudio_start_stream(dac);
    assert(err == 0);

    while (true)
    {
        usleep(1000000);
        if (!rtaudio_is_stream_running(dac))
        {
            break;
        }
    }

    fprintf(stderr, "Stopping audio streams\n");

    err = rtaudio_stop_stream(dac);
    assert(err == 0);
    if (rtaudio_is_stream_open(dac))
    {
        rtaudio_close_stream(dac);
    }

    fprintf(stderr, "Exiting\n");
    exit(EXIT_SUCCESS);
}
