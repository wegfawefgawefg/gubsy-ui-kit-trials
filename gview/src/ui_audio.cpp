#include "ui_audio.hpp"

#include <SDL3/SDL.h>
#include <sndfile.h>

#include <array>
#include <algorithm>
#include <cstdio>
#include <limits>
#include <random>
#include <string>
#include <vector>

struct UiAudio::Impl {
    struct Sound {
        SDL_AudioSpec spec{};
        std::vector<float> samples;
    };

    std::array<Sound, 3> sounds;
    SDL_AudioDeviceID device = 0;
    std::vector<SDL_AudioStream*> tracks;
    std::minstd_rand random{std::random_device{}()};
    std::size_t next_track = 0;
};

namespace {

std::string sound_path(int number) {
    return std::string(GVIEW_TRIAL_SOURCE_DIR) + "/assets/sounds/wood-block-" +
           std::to_string(number) + ".ogg";
}

} // namespace

UiAudio::UiAudio() : impl_(std::make_unique<Impl>()) {
    for (std::size_t index = 0; index < impl_->sounds.size(); ++index) {
        const std::string path = sound_path(static_cast<int>(index) + 1);
        SF_INFO info{};
        SNDFILE* file = sf_open(path.c_str(), SFM_READ, &info);
        if (!file) {
            std::fprintf(stderr, "UI sound load failed: %s: %s\n", path.c_str(), sf_strerror(0));
            continue;
        }
        Impl::Sound& sound = impl_->sounds[index];
        sound.spec = {SDL_AUDIO_F32, info.channels, info.samplerate};
        sound.samples.resize(static_cast<std::size_t>(info.frames) *
                             static_cast<std::size_t>(info.channels));
        sf_readf_float(file, sound.samples.data(), info.frames);
        sf_close(file);
        for (float& sample : sound.samples)
            sample *= 0.55f;
    }
    if (impl_->sounds.front().samples.empty()) return;
    impl_->device = SDL_OpenAudioDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, nullptr);
    if (impl_->device == 0) {
        std::fprintf(stderr, "UI audio disabled: %s\n", SDL_GetError());
        return;
    }
    for (int index = 0; index < 8; ++index) {
        SDL_AudioStream* track = SDL_CreateAudioStream(&impl_->sounds.front().spec, nullptr);
        if (!track) continue;
        if (!SDL_BindAudioStream(impl_->device, track)) {
            SDL_DestroyAudioStream(track);
            continue;
        }
        impl_->tracks.push_back(track);
    }
    SDL_ResumeAudioDevice(impl_->device);
}

UiAudio::~UiAudio() {
    if (!impl_) return;
    for (SDL_AudioStream* track : impl_->tracks)
        SDL_DestroyAudioStream(track);
    if (impl_->device != 0) SDL_CloseAudioDevice(impl_->device);
}

void UiAudio::play(gview::FeedbackEvent event) {
    if (impl_->tracks.empty()) return;
    const std::size_t sound_index = event == gview::FeedbackEvent::Move
                                        ? static_cast<std::size_t>(impl_->random() % 2U)
                                        : 2U;
    const Impl::Sound& sound = impl_->sounds[sound_index];
    if (sound.samples.empty()) return;
    SDL_AudioStream* track = nullptr;
    for (SDL_AudioStream* candidate : impl_->tracks) {
        if (SDL_GetAudioStreamQueued(candidate) == 0) {
            track = candidate;
            break;
        }
    }
    if (!track) {
        track = impl_->tracks[impl_->next_track++ % impl_->tracks.size()];
        SDL_ClearAudioStream(track);
    }
    const std::size_t byte_count = sound.samples.size() * sizeof(float);
    const int bytes = static_cast<int>(std::min(
        byte_count, static_cast<std::size_t>(std::numeric_limits<int>::max())));
    SDL_PutAudioStreamData(track, sound.samples.data(), bytes);
}
