#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include "Params.h"
#include "SynthDevice.h"
#include "DrumDevice.h"
#include "SamplerDevice.h"
#include "Effects.h"

class RackForgeProcessor : public juce::AudioProcessor
{
public:
    RackForgeProcessor();
    ~RackForgeProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "RackForge"; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 4.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return "Init"; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState apvts;

    // Données affichées par l'éditeur
    std::atomic<int>   currentStep { -1 };
    std::atomic<float> meterSynth { 0.0f }, meterDrums { 0.0f }, meterSampler { 0.0f }, meterMaster { 0.0f };
    std::atomic<float> hostBpm { 120.0f };

    PadSampler sampler;
    DrumMachine drums;
    double currentSampleRate = 44100.0;

    // --- Rack à modules libres : ordre des modules (instruments + effets) ---
    std::vector<int> getRackOrder() const;           // liste d'identifiants ModuleId
    void setRackOrder (const std::vector<int>& order);
    void rackAdd (int module);                       // ajoute en fin de rack (effets) ou avant le 1er effet (instruments)
    void rackRemove (int module);
    void rackMove (int module, int delta);           // delta = -1 (monter) / +1 (descendre)
    bool rackContains (int module) const;
    std::atomic<int> rackVersion { 0 };              // incrémenté à chaque changement (l'éditeur se reconstruit)
    bool hasAudioInput() const { return getTotalNumInputChannels() > 0; }
    // Déclenchement d'un pad depuis l'interface (lu par le thread audio)
    void triggerPadFromUI (int pad) { uiPadTrigger.store (pad); }

private:
    std::atomic<int> uiPadTrigger { -1 };
    SynthParams synthParams;
    juce::Synthesiser synth;
    DistortionFx dist;
    TempoDelayFx delay;
    ReverbFx reverb;
    CompressorFx comp;
    ChorusFx chorus;
    PhaserFx phaser;
    Eq3Fx eq;

    // Ordre du rack empaqueté : 4 bits par module, nombre de modules dans les 4 bits de poids fort (lecture atomique)
    std::atomic<juce::uint64> packedRack { 0 };
    static juce::uint64 packRack (const std::vector<int>& order);
    static std::vector<int> unpackRack (juce::uint64 packed);
    void processEffect (int module, juce::AudioBuffer<float>& buffer, double bpm);

    juce::AudioBuffer<float> synthBuf, drumBuf, samplerBuf, inputCopy;
    juce::MidiBuffer synthMidi, drumMidi, samplerMidi;

    std::atomic<float>* synthLevel = nullptr; std::atomic<float>* synthPan = nullptr; std::atomic<float>* synthMute = nullptr;
    std::atomic<float>* drumsLevel = nullptr; std::atomic<float>* drumsPan = nullptr; std::atomic<float>* drumsMute = nullptr;
    std::atomic<float>* samplerLevel = nullptr; std::atomic<float>* samplerPan = nullptr; std::atomic<float>* samplerMute = nullptr;
    std::atomic<float>* masterLevel = nullptr;

    static void mixInto (juce::AudioBuffer<float>& dst, const juce::AudioBuffer<float>& src, float level, float pan, bool mute);
    static float peakOf (const juce::AudioBuffer<float>& b);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RackForgeProcessor)
};
