#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include "Params.h"

// Boîte à rythmes "Pulse-8" : 8 sons synthétisés (pas d'échantillons), 16 pas
class DrumVoice
{
public:
    void prepare (double sr_) { sr = (float) sr_; active = false; }

    void trigger (int type_, float vel, float tune_, float decay_)
    {
        type = type_; velocity = vel; tune = tune_; decay = decay_;
        t = 0.0f; phase = 0.0f; lp = 0.0f; lp2 = 0.0f; active = true;
    }

    bool isActive() const { return active; }
    void stop() { active = false; }

    float render()
    {
        if (! active) return 0.0f;
        const float dt = 1.0f / sr;
        float out = 0.0f, env = 0.0f;

        switch (type)
        {
            case 0: // Kick
            {
                const float f = 52.0f * tune * (1.0f + 6.0f * std::exp (-t * 45.0f));
                phase += f * dt;
                env = std::exp (-t / (0.08f + decay * 0.5f));
                out = std::sin (juce::MathConstants<float>::twoPi * phase) * env;
                out = std::tanh (out * 1.8f);
                break;
            }
            case 1: // Snare
            {
                const float f = 185.0f * tune * (1.0f + 1.5f * std::exp (-t * 60.0f));
                phase += f * dt;
                const float tone = std::sin (juce::MathConstants<float>::twoPi * phase) * std::exp (-t * 28.0f);
                env = std::exp (-t / (0.04f + decay * 0.3f));
                const float n = highpass (noise(), 1800.0f);
                out = tone * 0.5f + n * env * 0.8f;
                break;
            }
            case 2: // Charley fermé
            {
                env = std::exp (-t / (0.015f + decay * 0.08f));
                out = highpass (metal(), 7000.0f * tune) * env;
                break;
            }
            case 3: // Charley ouvert
            {
                env = std::exp (-t / (0.08f + decay * 0.7f));
                out = highpass (metal(), 6500.0f * tune) * env * 0.9f;
                break;
            }
            case 4: // Clap
            {
                float burst = 0.0f;
                for (int k = 0; k < 3; ++k)
                {
                    const float tk = t - k * 0.011f;
                    if (tk >= 0.0f) burst += std::exp (-tk * 300.0f);
                }
                env = std::exp (-t / (0.03f + decay * 0.25f));
                const float n = bandpass (noise(), 1200.0f);
                out = n * (burst * 0.6f + env * 0.5f);
                break;
            }
            case 5: // Tom grave
            case 6: // Tom aigu
            {
                const float base = (type == 5 ? 95.0f : 165.0f) * tune;
                const float f = base * (1.0f + 1.2f * std::exp (-t * 30.0f));
                phase += f * dt;
                env = std::exp (-t / (0.08f + decay * 0.4f));
                out = std::sin (juce::MathConstants<float>::twoPi * phase) * env
                    + highpass (noise(), 3000.0f) * std::exp (-t * 80.0f) * 0.15f;
                break;
            }
            default: // Rim
            {
                phase += 820.0f * tune * dt;
                env = std::exp (-t / (0.004f + decay * 0.03f));
                const float sq = (std::fmod (phase, 1.0f) < 0.5f ? 1.0f : -1.0f);
                out = (sq * 0.5f + highpass (noise(), 4000.0f) * 0.5f) * env;
                break;
            }
        }

        t += dt;
        if (t > 0.02f && env < 0.0005f) active = false;
        return out * velocity;
    }

private:
    float noise() { return rng.nextFloat() * 2.0f - 1.0f; }

    // Bruit "métallique" : somme de carrés inharmoniques (style 808)
    float metal()
    {
        static const float ratios[6] = { 1.0f, 1.342f, 1.2312f, 1.6532f, 1.9523f, 2.1523f };
        float s = 0.0f;
        for (int k = 0; k < 6; ++k)
        {
            mphase[k] += 320.0f * ratios[k] * tune / sr;
            if (mphase[k] >= 1.0f) mphase[k] -= 1.0f;
            s += (mphase[k] < 0.5f ? 1.0f : -1.0f);
        }
        return s / 6.0f;
    }

    float highpass (float x, float fc)
    {
        const float a = juce::jlimit (0.0f, 0.999f, juce::MathConstants<float>::twoPi * fc / sr);
        lp += a * (x - lp);
        return x - lp;
    }

    float bandpass (float x, float fc)
    {
        const float a = juce::jlimit (0.0f, 0.999f, juce::MathConstants<float>::twoPi * fc / sr);
        lp += a * (x - lp);
        lp2 += a * (lp - lp2);
        return lp - lp2;
    }

    int type = 0;
    float sr = 44100.0f, t = 0.0f, phase = 0.0f, velocity = 1.0f, tune = 1.0f, decay = 0.4f;
    float lp = 0.0f, lp2 = 0.0f;
    float mphase[6] = { 0, 0.1f, 0.2f, 0.3f, 0.4f, 0.5f };
    bool active = false;
    juce::Random rng;
};

class DrumMachine
{
public:
    void attach (juce::AudioProcessorValueTreeState& s)
    {
        run = s.getRawParameterValue (ID::drumRun);
        swing = s.getRawParameterValue (ID::drumSwing);
        pattern = s.getRawParameterValue (ID::drumPattern);
        chain = s.getRawParameterValue (ID::drumChain);
        for (int t = 0; t < kNumDrumTracks; ++t)
        {
            level[t] = s.getRawParameterValue (ID::drumLevel (t));
            tune[t]  = s.getRawParameterValue (ID::drumTune (t));
            decay[t] = s.getRawParameterValue (ID::drumDecay (t));
            for (int pat = 0; pat < kNumPatterns; ++pat)
                for (int st = 0; st < kNumSteps; ++st)
                    steps[pat][t][st] = s.getRawParameterValue (ID::step (pat, t, st));
        }
    }

    // Motif en cours de lecture (en mode chaîne, suit la mesure de l'hôte)
    std::atomic<int> playingPattern { 0 };

    void prepare (double sr)
    {
        sampleRate = sr;
        for (auto& v : voices) v.prepare (sr);
        lastStep = -1; internalPpq = 0.0;
    }

    void triggerTrack (int t, float vel)
    {
        if (t < 0 || t >= kNumDrumTracks) return;
        // Charley fermé coupe le charley ouvert
        if (t == 2) voices[3].stop();
        voices[t].trigger (t, vel * *level[t], *tune[t], *decay[t]);
    }

    // Rend le bloc : MIDI (notes 36..43) + séquenceur synchronisé sur l'hôte
    void process (juce::AudioBuffer<float>& out, const juce::MidiBuffer& midi,
                  bool hostPlaying, bool hasHostPpq, double hostPpq, double bpm, std::atomic<int>& stepOut)
    {
        const int num = out.getNumSamples();
        const double ppqPerSample = bpm / 60.0 / sampleRate;
        const bool seqOn = *run > 0.5f;
        const bool useHost = hostPlaying && hasHostPpq;
        if (! useHost && ! seqOn) { lastStep = -1; stepOut.store (-1); }

        auto midiIt = midi.begin();
        double ppq = useHost ? hostPpq : internalPpq;
        const double sw = (double) *swing * 0.33;

        for (int i = 0; i < num; ++i)
        {
            while (midiIt != midi.end() && (*midiIt).samplePosition <= i)
            {
                const auto m = (*midiIt).getMessage();
                if (m.isNoteOn())
                {
                    const int t = m.getNoteNumber() - 36;
                    if (t >= 0 && t < kNumDrumTracks) triggerTrack (t, m.getFloatVelocity());
                }
                ++midiIt;
            }

            if (seqOn)
            {
                const double pos = ppq / 0.25;
                int k = (int) std::floor (pos);
                const double frac = pos - k;
                if ((k & 1) && frac < sw) --k;          // swing : retarde les pas impairs
                if (k != lastStep)
                {
                    lastStep = k;
                    const int s = ((k % kNumSteps) + kNumSteps) % kNumSteps;
                    // motif : fixe, ou enchaîné A-B-C-D (une mesure de 16 pas chacun)
                    int pat = juce::jlimit (0, kNumPatterns - 1, (int) *pattern);
                    if (*chain > 0.5f)
                    {
                        const int bar = (int) std::floor ((double) k / kNumSteps);
                        pat = ((bar % kNumPatterns) + kNumPatterns) % kNumPatterns;
                    }
                    playingPattern.store (pat);
                    stepOut.store (s);
                    for (int t = 0; t < kNumDrumTracks; ++t)
                        if (*steps[pat][t][s] > 0.5f) triggerTrack (t, 1.0f);
                }
                ppq += ppqPerSample;
            }

            float s = 0.0f;
            for (auto& v : voices) s += v.render();
            s *= 0.6f;
            for (int ch = 0; ch < out.getNumChannels(); ++ch)
                out.addSample (ch, i, s);
        }

        if (! useHost) internalPpq = ppq; else internalPpq = 0.0;
    }

private:
    double sampleRate = 44100.0, internalPpq = 0.0;
    int lastStep = -1;
    DrumVoice voices[kNumDrumTracks];
    std::atomic<float>* run = nullptr; std::atomic<float>* swing = nullptr;
    std::atomic<float>* pattern = nullptr; std::atomic<float>* chain = nullptr;
    std::atomic<float>* level[kNumDrumTracks] {};
    std::atomic<float>* tune[kNumDrumTracks] {};
    std::atomic<float>* decay[kNumDrumTracks] {};
    std::atomic<float>* steps[kNumPatterns][kNumDrumTracks][kNumSteps] {};
};
