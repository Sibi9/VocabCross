#pragma once
#include "raylib.h"
#include <cmath>
#include <vector>

class AudioManager {
public:
    Sound snd_pop{};
    Sound snd_fanfare{};
    Sound snd_chime{};
    Sound snd_click{};
    Sound snd_whoosh{};
    bool enabled = true;
    bool initialized = false;

    static Sound makeSound(std::vector<short>& buffer, int rate, int count) {
        Wave w{};
        w.frameCount = (unsigned int)count;
        w.sampleRate = (unsigned int)rate;
        w.sampleSize = 16;
        w.channels = 1;
        w.data = buffer.data();
        Sound s = LoadSoundFromWave(w);
        // Do NOT call UnloadWave(w): the samples live in the std::vector and
        // LoadSoundFromWave has already copied them into the audio backend.
        return s;
    }

    void init() {
        if (initialized) return;
        InitAudioDevice();
        initialized = true;
        int rate = 44100;

        // 1. Pop (letter selection)
        {
            float dur = 0.08f;
            int count = (int)(rate * dur);
            std::vector<short> buf(count);
            for (int i = 0; i < count; i++) {
                float t = (float)i / rate;
                float f = 550.0f + 350.0f * (t / dur);
                float env = 1.0f - (float)i / count;
                buf[i] = (short)(sinf(2.0f * PI * f * t) * 28000.0f * env);
            }
            snd_pop = makeSound(buf, rate, count);
        }
        // 2. Chime (hint)
        {
            float dur = 0.35f;
            int count = (int)(rate * dur);
            std::vector<short> buf(count);
            for (int i = 0; i < count; i++) {
                float t = (float)i / rate;
                float env = 1.0f - (float)i / count;
                float val = sinf(2.0f * PI * 1318.51f * t) * 0.6f + sinf(2.0f * PI * 1760.0f * t) * 0.4f;
                buf[i] = (short)(val * 24000.0f * env);
            }
            snd_chime = makeSound(buf, rate, count);
        }
        // 3. Click
        {
            float dur = 0.04f;
            int count = (int)(rate * dur);
            std::vector<short> buf(count);
            for (int i = 0; i < count; i++) {
                float env = 1.0f - (float)i / count;
                buf[i] = (short)(((rand() % 32767) - 16384) * env * 0.6f);
            }
            snd_click = makeSound(buf, rate, count);
        }
        // 4. Whoosh
        {
            float dur = 0.2f;
            int count = (int)(rate * dur);
            std::vector<short> buf(count);
            for (int i = 0; i < count; i++) {
                float t = (float)i / rate;
                float env = sinf(PI * (t / dur));
                float f = 300.0f - 150.0f * (t / dur);
                buf[i] = (short)(sinf(2.0f * PI * f * t) * 20000.0f * env);
            }
            snd_whoosh = makeSound(buf, rate, count);
        }
        // 5. Victory arpeggio (C-E-G-C)
        {
            float dur = 0.6f;
            int count = (int)(rate * dur);
            std::vector<short> buf(count);
            float notes[] = { 523.25f, 659.25f, 783.99f, 1046.50f };
            float seg = dur / 4.0f;
            for (int i = 0; i < count; i++) {
                float t = (float)i / rate;
                int seg_idx = (int)(t / seg);
                if (seg_idx > 3) seg_idx = 3;
                float seg_t = t - (float)seg_idx * seg;
                float env = (seg_idx == 3) ? (1.0f - (seg_t / (seg * 1.5f))) : 1.0f;
                if (env < 0) env = 0;
                float f = notes[seg_idx];
                buf[i] = (short)(sinf(2.0f * PI * f * t) * 25000.0f * env);
            }
            snd_fanfare = makeSound(buf, rate, count);
        }
    }

    void playPop() { if (enabled && initialized) PlaySound(snd_pop); }
    void playChime() { if (enabled && initialized) PlaySound(snd_chime); }
    void playClick() { if (enabled && initialized) PlaySound(snd_click); }
    void playFanfare() { if (enabled && initialized) PlaySound(snd_fanfare); }
    void playWhoosh() { if (enabled && initialized) PlaySound(snd_whoosh); }

    void close() {
        if (!initialized) return;
        UnloadSound(snd_pop);
        UnloadSound(snd_fanfare);
        UnloadSound(snd_chime);
        UnloadSound(snd_click);
        UnloadSound(snd_whoosh);
        CloseAudioDevice();
        initialized = false;
    }
};

inline AudioManager gAudio;
