#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include "Params.h"

// Paramètres partagés par toutes les voix (lus en temps réel via les atomics de l'APVTS)
struct SynthParams
{
    std::atomic<float>* osc1Wave = nullptr;
    std::atomic<float>* osc2Wave = nullptr;
    std::atomic<float>* osc2Detune = nullptr;
    std::atomic<float>* osc2Oct = nullptr;
    std::atomic<float>* oscMix = nullptr;
    std::atomic<float>* subLevel = nullptr;
    std::atomic<float>* cutoff = nullptr;
    std::atomic<float>* reso = nullptr;
    std::atomic<float>* fEnvAmt = nullptr;
    std::atomic<float>* ampA = nullptr; std::atomic<float>* ampD = nullptr;
    std::atomic<float>* ampS = nullptr; std::atomic<float>* ampR = nullptr;
    std::atomic<float>* filA = nullptr; std::atomic<float>* filD = nullptr;
    std::atomic<float>* filS = nullptr; std::atomic<float>* filR = nullptr;
    std::atomic<float>* lfoRate = nullptr;
    std::atomic<float>* lfoPitch = nullptr;
    std::atomic<float>* lfoCutoff = nullptr;

    void attach (juce::AudioProcessorValueTreeState& s)
    {
        osc1Wave = s.getRawParameterValue (ID::osc1Wave);
        osc2Wave = s.getRawParameterValue (ID::osc2Wave);
        osc2Detune = s.getRawParameterValue (ID::osc2Detune);
        osc2Oct = s.getRawParameterValue (ID::osc2Oct);
        oscMix = s.getRawParameterValue (ID::oscMix);
        subLevel = s.getRawParameterValue (ID::subLevel);
        cutoff = s.getRawParameterValue (ID::cutoff);
        reso = s.getRawParameterValue (ID::reso);
        fEnvAmt = s.getRawParameterValue (ID::fEnvAmt);
        ampA = s.getRawParameterValue (ID::ampA); ampD = s.getRawParameterValue (ID::ampD);
        ampS = s.getRawParameterValue (ID::ampS); ampR = s.getRawParameterValue (ID::ampR);
        filA = s.getRawParameterValue (ID::filA); filD = s.getRawParameterValue (ID::filD);
        filS = s.getRawParameterValue (ID::filS); filR = s.getRawParameterValue (ID::filR);
        lfoRate = s.getRawParameterValue (ID::lfoRate);
        lfoPitch = s.getRawParameterValue (ID::lfoPitch);
        lfoCutoff = s.getRawParameterValue (ID::lfoCutoff);
    }
};

// Oscillateur à PolyBLEP (dent de scie / carré sans aliasing marqué)
class BlepOscillator
{
public:
    void setFrequency (float hz, double sr) { inc = (float) (hz / sr); }
    void reset (float p = 0.0f) { phase = p; }

    float next (int wave)
    {
        float t = phase, v = 0.0f;
        switch (wave)
        {
            case 0: v = 2.0f * t - 1.0f - blep (t); break;                         // scie
            case 1: v = (t < 0.5f ? 1.0f : -1.0f) + blep (t) - blep (std::fmod (t + 0.5f, 1.0f)); break; // carré
            case 2: v = 4.0f * std::abs (t - 0.5f) - 1.0f; break;                  // triangle
            default: v = std::sin (juce::MathConstants<float>::twoPi * t); break;  // sinus
        }
        phase += inc;
        if (phase >= 1.0f) phase -= 1.0f;
        return v;
    }

private:
    float blep (float t) const
    {
        if (t < inc)            { t /= inc; return t + t - t * t - 1.0f; }
        if (t > 1.0f - inc)     { t = (t - 1.0f) / inc; return t * t + t + t + 1.0f; }
        return 0.0f;
    }
    float phase = 0.0f, inc = 0.0f;
};

struct SynthSound : public juce::SynthesiserSound
{
    bool appliesToNote (int) override { return true; }
    bool appliesToChannel (int) override { return true; }
};

class SynthVoice : public juce::SynthesiserVoice
{
public:
    explicit SynthVoice (const SynthParams& p) : params (p) {}

    void prepare (double sr, int blockSize)
    {
        juce::dsp::ProcessSpec spec { sr, (juce::uint32) blockSize, 1 };
        filter.prepare (spec);
        filter.setType (juce::dsp::StateVariableTPTFilterType::lowpass);
        ampEnv.setSampleRate (sr);
        filEnv.setSampleRate (sr);
        lfoInc = 0.0f;
    }

    bool canPlaySound (juce::SynthesiserSound* s) override { return dynamic_cast<SynthSound*> (s) != nullptr; }

    void startNote (int midiNote, float velocity, juce::SynthesiserSound*, int) override
    {
        baseFreq = (float) juce::MidiMessage::getMidiNoteInHertz (midiNote);
        level = 0.25f * (0.3f + 0.7f * velocity);
        ampEnv.setParameters ({ *params.ampA, *params.ampD, *params.ampS, *params.ampR });
        filEnv.setParameters ({ *params.filA, *params.filD, *params.filS, *params.filR });
        ampEnv.noteOn();
        filEnv.noteOn();
        filter.reset();
        osc1.reset (0.0f); osc2.reset (0.37f); sub.reset (0.0f);
    }

    void stopNote (float, bool allowTailOff) override
    {
        if (allowTailOff) { ampEnv.noteOff(); filEnv.noteOff(); }
        else { ampEnv.reset(); filEnv.reset(); clearCurrentNote(); }
    }

    void pitchWheelMoved (int v) override { bend = (v - 8192) / 8192.0f * 2.0f; }
    void controllerMoved (int, int) override {}

    void renderNextBlock (juce::AudioBuffer<float>& out, int start, int num) override
    {
        if (! ampEnv.isActive()) { clearCurrentNote(); return; }

        const double sr = getSampleRate();
        const int w1 = (int) *params.osc1Wave, w2 = (int) *params.osc2Wave;
        const float detune = std::pow (2.0f, *params.osc2Detune / 1200.0f);
        const float oct = std::pow (2.0f, (float) ((int) *params.osc2Oct - 1));
        const float mix = *params.oscMix, subLvl = *params.subLevel;
        const float baseCut = *params.cutoff, fEnvAmt = *params.fEnvAmt;
        const float lfoPitch = *params.lfoPitch, lfoCut = *params.lfoCutoff;
        lfoInc = (float) (*params.lfoRate / sr);
        filter.setResonance (0.5f + *params.reso * 9.0f);

        for (int i = 0; i < num; ++i)
        {
            const float lfo = std::sin (juce::MathConstants<float>::twoPi * lfoPhase);
            lfoPhase += lfoInc; if (lfoPhase >= 1.0f) lfoPhase -= 1.0f;

            const float pitchMod = std::pow (2.0f, (bend + lfo * lfoPitch * 0.5f) / 12.0f);
            const float f = baseFreq * pitchMod;
            osc1.setFrequency (f, sr);
            osc2.setFrequency (f * detune * oct, sr);
            sub.setFrequency (f * 0.5f, sr);

            float s = osc1.next (w1) * (1.0f - mix) + osc2.next (w2) * mix + sub.next (3) * subLvl;

            const float fe = filEnv.getNextSample();
            float cut = baseCut * std::pow (2.0f, fe * fEnvAmt * 5.0f + lfo * lfoCut * 3.0f);
            cut = juce::jlimit (20.0f, (float) (sr * 0.45), cut);
            filter.setCutoffFrequency (cut);
            s = filter.processSample (0, s);

            s *= ampEnv.getNextSample() * level;

            for (int ch = 0; ch < out.getNumChannels(); ++ch)
                out.addSample (ch, start + i, s);

            if (! ampEnv.isActive()) { clearCurrentNote(); break; }
        }
    }

private:
    const SynthParams& params;
    BlepOscillator osc1, osc2, sub;
    juce::dsp::StateVariableTPTFilter<float> filter;
    juce::ADSR ampEnv, filEnv;
    float baseFreq = 440.0f, level = 0.0f, bend = 0.0f;
    float lfoPhase = 0.0f, lfoInc = 0.0f;
};
