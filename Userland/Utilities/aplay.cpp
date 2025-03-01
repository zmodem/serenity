/*
 * Copyright (c) 2018-2020, Andreas Kling <kling@serenityos.org>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <LibAudio/Buffer.h>
#include <LibAudio/ClientConnection.h>
#include <LibAudio/Loader.h>
#include <LibCore/ArgsParser.h>
#include <LibCore/EventLoop.h>
#include <stdio.h>

int main(int argc, char** argv)
{
    const char* path = nullptr;
    bool should_loop = false;

    Core::ArgsParser args_parser;
    args_parser.add_positional_argument(path, "Path to audio file", "path");
    args_parser.add_option(should_loop, "Loop playback", "loop", 'l');
    args_parser.parse(argc, argv);

    Core::EventLoop loop;

    auto audio_client = Audio::ClientConnection::construct();
    NonnullRefPtr<Audio::Loader> loader = Audio::Loader::create(path);
    if (loader->has_error()) {
        fprintf(stderr, "Failed to load audio file: %s\n", loader->error_string());
        return 1;
    }

    printf("\033[34;1m Playing\033[0m: %s\n", path);
    printf("\033[34;1m  Format\033[0m: %u Hz, %u-bit, %s\n",
        loader->sample_rate(),
        loader->bits_per_sample(),
        loader->num_channels() == 1 ? "Mono" : "Stereo");
    printf("\033[34;1mProgress\033[0m: \033[s");
    for (;;) {
        auto samples = loader->get_more_samples();
        if (samples) {
            printf("\033[u");
            printf("%d/%d", loader->loaded_samples(), loader->total_samples());
            fflush(stdout);
            audio_client->enqueue(*samples);
        } else if (should_loop) {
            loader->reset();
        } else if (audio_client->get_remaining_samples()) {
            sleep(1);
        } else {
            break;
        }
    }
    printf("\n");
    return 0;
}
