#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>
#include "Params.h"

// ---------------------------------------------------------------- Distorsion
class DistortionFx
{
public:
    void attach (juce::AudioProcessorValueTreeState& s)
    {
        on = s.getRawParameterValue (ID::distOn);
        drive = s.getRawParameterValue (ID::distDrive);
        tone = s.getRawParameterValue (ID::distTone);
        mix = s.getRawParameterValue (ID::distMix);
    }
    void prepare (double sr_) { sr = (float) sr_; lp[0] = lp[1] = 0.0f; }

    void process (juce::AudioBuffer<float>& b)
    {
        if (*on < 0.5f) return;
        const float g = 1.0f + *drive * 30.0f;
        const float m = *mix;
        const float a = juce::jlimit (0.0f, 0.999f, juce::MathConstants<float>::twoPi * *tone / sr);
        const float comp = 1.0f / std::tanh (juce::jmin (g, 4.0f));
        for (int ch = 0; ch < juce::jmin (2, b.getNumChannels()); ++ch)
        {
            auto* d = b.getWritePointer (ch);
            for (int i = 0; i < b.getNumSamples(); ++i)
            {
                float w = std::tanh (d[i] * g) * comp;
                lp[ch] += a * (w - lp[ch]);
                d[i] = d[i] * (1.0f - m) + lp[ch] * m;
            }
        }
    }
private:
    float sr = 44100.0f, lp[2] { 0, 0 };
    std::atomic<float>* on = nullptr; std::atomic<float>* drive = nullptr;
    std::atomic<float>* tone = nullptr; std::atomic<float>* mix = nullptr;
};

// ---------------------------------------------------------------- Delay synchronisé au tempo
class TempoDelayFx
{
public:
    void attach (juce::AudioProcessorValueTreeState& s)
    {
        on = s.getRawParameterValue (ID::dlyOn);
        time = s.getRawParameterValue (ID::dlyTime);
        feedback = s.getRawParameterValue (ID::dlyFeedback);
        mix = s.getRawParameterValue (ID::dlyMix);
    }
    void prepare (double sr_, int block)
    {
        sr = sr_;
        juce::dsp::ProcessSpec spec { sr, (juce::uint32) block, 2 };
        line.setMaximumDelayInSamples ((int) (sr * 4.0) + 1);
        line.prepare (spec);
        line.reset();
        smoothed.reset (sr, 0.05);
    }

    void process (juce::AudioBuffer<float>& b, double bpm)
    {
        if (*on < 0.5f) { line.reset(); return; }
        static const double beats[6] = { 1.0, 0.5, 0.75, 1.0 / 3.0, 0.25, 1.0 / 6.0 };
        const int idx = juce::jlimit (0, 5, (int) *time);
        const double seconds = beats[idx] * 60.0 / juce::jmax (20.0, bpm);
        smoothed.setTargetValue ((float) juce::jlimit (1.0, sr * 3.9, seconds * sr));
        const float fb = *feedback, m = *mix;
        const int chans = juce::jmin (2, b.getNumChannels());

        for (int i = 0; i < b.getNumSamples(); ++i)
        {
            const float d = smoothed.getNextValue();
            line.setDelay (d);
            for (int ch = 0; ch < chans; ++ch)
            {
                auto* p = b.getWritePointer (ch);
                const float wet = line.popSample (ch);
                line.pushSample (ch, p[i] + wet * fb);
                p[i] = p[i] * (1.0f - m * 0.5f) + wet * m;
            }
        }
    }
private:
    double sr = 44100.0;
    juce::dsp::DelayLine<float, juce::dsp::DelayLineInterpolationTypes::Linear> line { 192000 };
    juce::SmoothedValue<float> smoothed;
    std::atomic<float>* on = nullptr; std::atomic<float>* time = nullptr;
    std::atomic<float>* feedback = nullptr; std::atomic<float>* mix = nullptr;
};

// ---------------------------------------------------------------- Réverbération
class ReverbFx
{
public:
    void attach (juce::AudioProcessorValueTreeState& s)
    {
        on = s.getRawParameterValue (ID::revOn);
        size = s.getRawParameterValue (ID::revSize);
        damp = s.getRawParameterValue (ID::revDamp);
        width = s.getRawParameterValue (ID::revWidth);
        mix = s.getRawParameterValue (ID::revMix);
    }
    void prepare (double sr) { reverb.setSampleRate (sr); reverb.reset(); }

    void process (juce::AudioBuffer<float>& b)
    {
        if (*on < 0.5f) return;
        juce::Reverb::Parameters p;
        p.roomSize = *size; p.damping = *damp; p.width = *width;
        p.wetLevel = *mix; p.dryLevel = 1.0f - *mix * 0.5f; p.freezeMode = 0.0f;
        reverb.setParameters (p);
        if (b.getNumChannels() >= 2)
            reverb.processStereo (b.getWritePointer (0), b.getWritePointer (1), b.getNumSamples());
        else
            reverb.processMono (b.getWritePointer (0), b.getNumSamples());
    }
private:
    juce::Reverb reverb;
    std::atomic<float>* on = nullptr; std::atomic<float>* size = nullptr; std::atomic<float>* damp = nullptr;
    std::atomic<float>* width = nullptr; std::atomic<float>* mix = nullptr;
};

// ---------------------------------------------------------------- Chorus
class ChorusFx
{
public:
    void attach (juce::AudioProcessorValueTreeState& s)
    {
        on = s.getRawParameterValue (ID::choOn);
        rate = s.getRawParameterValue (ID::choRate);
        depth = s.getRawParameterValue (ID::choDepth);
        delay = s.getRawParameterValue (ID::choDelay);
        feedback = s.getRawParameterValue (ID::choFeedback);
        mix = s.getRawParameterValue (ID::choMix);
    }
    void prepare (double sr, int block)
    {
        juce::dsp::ProcessSpec spec { sr, (juce::uint32) block, 2 };
        chorus.prepare (spec);
        chorus.reset();
    }
    void process (juce::AudioBuffer<float>& b)
    {
        if (*on < 0.5f || b.getNumChannels() < 2) return;
        chorus.setRate (*rate);
        chorus.setDepth (*depth);
        chorus.setCentreDelay (*delay);
        chorus.setFeedback (*feedback);
        chorus.setMix (*mix);
        juce::dsp::AudioBlock<float> block (b);
        juce::dsp::ProcessContextReplacing<float> ctx (block);
        chorus.process (ctx);
    }
private:
    juce::dsp::Chorus<float> chorus;
    std::atomic<float>* on = nullptr; std::atomic<float>* rate = nullptr; std::atomic<float>* depth = nullptr;
    std::atomic<float>* delay = nullptr; std::atomic<float>* feedback = nullptr; std::atomic<float>* mix = nullptr;
};

// ---------------------------------------------------------------- Phaser
class PhaserFx
{
public:
    void attach (juce::AudioProcessorValueTreeState& s)
    {
        on = s.getRawParameterValue (ID::phaOn);
        rate = s.getRawParameterValue (ID::phaRate);
        depth = s.getRawParameterValue (ID::phaDepth);
        centre = s.getRawParameterValue (ID::phaCentre);
        feedback = s.getRawParameterValue (ID::phaFeedback);
        mix = s.getRawParameterValue (ID::phaMix);
    }
    void prepare (double sr, int block)
    {
        juce::dsp::ProcessSpec spec { sr, (juce::uint32) block, 2 };
        phaser.prepare (spec);
        phaser.reset();
    }
    void process (juce::AudioBuffer<float>& b)
    {
        if (*on < 0.5f || b.getNumChannels() < 2) return;
        phaser.setRate (*rate);
        phaser.setDepth (*depth);
        phaser.setCentreFrequency (*centre);
        phaser.setFeedback (*feedback);
        phaser.setMix (*mix);
        juce::dsp::AudioBlock<float> block (b);
        juce::dsp::ProcessContextReplacing<float> ctx (block);
        phaser.process (ctx);
    }
private:
    juce::dsp::Phaser<float> phaser;
    std::atomic<float>* on = nullptr; std::atomic<float>* rate = nullptr; std::atomic<float>* depth = nullptr;
    std::atomic<float>* centre = nullptr; std::atomic<float>* feedback = nullptr; std::atomic<float>* mix = nullptr;
};

// ---------------------------------------------------------------- Égaliseur 3 bandes
class Eq3Fx
{
public:
    void attach (juce::AudioProcessorValueTreeState& s)
    {
        on = s.getRawParameterValue (ID::eqOn);
        lowGain = s.getRawParameterValue (ID::eqLowGain);   lowFreq = s.getRawParameterValue (ID::eqLowFreq);
        midGain = s.getRawParameterValue (ID::eqMidGain);   midFreq = s.getRawParameterValue (ID::eqMidFreq);
        midQ = s.getRawParameterValue (ID::eqMidQ);
        highGain = s.getRawParameterValue (ID::eqHighGain); highFreq = s.getRawParameterValue (ID::eqHighFreq);
    }
    void prepare (double sr_, int block)
    {
        sr = sr_;
        juce::dsp::ProcessSpec spec { sr, (juce::uint32) block, 2 };
        for (auto* f : { &low, &mid, &high }) { f->prepare (spec); f->reset(); }
        last = {};
    }
    void process (juce::AudioBuffer<float>& b)
    {
        if (*on < 0.5f) return;
        const Settings cur { *lowGain, *lowFreq, *midGain, *midFreq, *midQ, *highGain, *highFreq };
        if (std::memcmp (&cur, &last, sizeof (Settings)) != 0)
        {
            last = cur;
            using C = juce::dsp::IIR::Coefficients<float>;
            *low.state  = *C::makeLowShelf  (sr, cur.lf, 0.707f, juce::Decibels::decibelsToGain (cur.lg));
            *mid.state  = *C::makePeakFilter (sr, cur.mf, cur.mq, juce::Decibels::decibelsToGain (cur.mg));
            *high.state = *C::makeHighShelf (sr, cur.hf, 0.707f, juce::Decibels::decibelsToGain (cur.hg));
        }
        juce::dsp::AudioBlock<float> block (b);
        juce::dsp::ProcessContextReplacing<float> ctx (block);
        low.process (ctx); mid.process (ctx); high.process (ctx);
    }
private:
    struct Settings { float lg = 0, lf = 0, mg = 0, mf = 0, mq = 0, hg = 0, hf = 0; };
    Settings last;
    double sr = 44100.0;
    using Filter = juce::dsp::ProcessorDuplicator<juce::dsp::IIR::Filter<float>, juce::dsp::IIR::Coefficients<float>>;
    Filter low, mid, high;
    std::atomic<float>* on = nullptr;
    std::atomic<float>* lowGain = nullptr; std::atomic<float>* lowFreq = nullptr;
    std::atomic<float>* midGain = nullptr; std::atomic<float>* midFreq = nullptr; std::atomic<float>* midQ = nullptr;
    std::atomic<float>* highGain = nullptr; std::atomic<float>* highFreq = nullptr;
};

// ---------------------------------------------------------------- Compresseur
class CompressorFx
{
public:
    void attach (juce::AudioProcessorValueTreeState& s)
    {
        on = s.getRawParameterValue (ID::compOn);
        thresh = s.getRawParameterValue (ID::compThresh);
        ratio = s.getRawParameterValue (ID::compRatio);
        attack = s.getRawParameterValue (ID::compAttack);
        release = s.getRawParameterValue (ID::compRelease);
        makeup = s.getRawParameterValue (ID::compMakeup);
    }
    void prepare (double sr, int block)
    {
        juce::dsp::ProcessSpec spec { sr, (juce::uint32) block, 2 };
        comp.prepare (spec);
        gain.prepare (spec);
        gain.setRampDurationSeconds (0.02);
    }

    void process (juce::AudioBuffer<float>& b)
    {
        if (*on < 0.5f) return;
        comp.setThreshold (*thresh);
        comp.setRatio (*ratio);
        comp.setAttack (*attack);
        comp.setRelease (*release);
        gain.setGainDecibels (*makeup);
        juce::dsp::AudioBlock<float> block (b);
        juce::dsp::ProcessContextReplacing<float> ctx (block);
        comp.process (ctx);
        gain.process (ctx);
    }
private:
    juce::dsp::Compressor<float> comp;
    juce::dsp::Gain<float> gain;
    std::atomic<float>* on = nullptr; std::atomic<float>* thresh = nullptr; std::atomic<float>* ratio = nullptr;
    std::atomic<float>* attack = nullptr; std::atomic<float>* release = nullptr; std::atomic<float>* makeup = nullptr;
};
