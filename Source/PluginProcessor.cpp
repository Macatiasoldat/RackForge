#include "PluginProcessor.h"
#include "PluginEditor.h"

RackForgeProcessor::RackForgeProcessor()
    : AudioProcessor (BusesProperties()
                          .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                          .withInput ("Rec In", juce::AudioChannelSet::stereo(), false)),   // entrée optionnelle pour l'enregistreur PADLAB
      apvts (*this, nullptr, "RackForge", createParameterLayout())
{
    synthParams.attach (apvts);
    for (int i = 0; i < 8; ++i)
        synth.addVoice (new SynthVoice (synthParams));
    synth.addSound (new SynthSound());

    drums.attach (apvts);
    sampler.attach (apvts);
    dist.attach (apvts);
    delay.attach (apvts);
    reverb.attach (apvts);
    comp.attach (apvts);
    chorus.attach (apvts);
    phaser.attach (apvts);
    eq.attach (apvts);

    // Rack par défaut
    std::vector<int> def;
    for (auto key : juce::StringArray::fromTokens (kDefaultRack, ",", ""))
        for (int m = 0; m < kNumModules; ++m)
            if (key == kModuleKeys[m]) def.push_back (m);
    packedRack.store (packRack (def));

    synthLevel = apvts.getRawParameterValue (ID::synthLevel);
    synthPan   = apvts.getRawParameterValue (ID::synthPan);
    synthMute  = apvts.getRawParameterValue (ID::synthMute);
    drumsLevel = apvts.getRawParameterValue (ID::drumsLevel);
    drumsPan   = apvts.getRawParameterValue (ID::drumsPan);
    drumsMute  = apvts.getRawParameterValue (ID::drumsMute);
    samplerLevel = apvts.getRawParameterValue (ID::samplerLevel);
    samplerPan   = apvts.getRawParameterValue (ID::samplerPan);
    samplerMute  = apvts.getRawParameterValue (ID::samplerMute);
    masterLevel = apvts.getRawParameterValue (ID::masterLevel);
}

void RackForgeProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    synth.setCurrentPlaybackSampleRate (sampleRate);
    for (int i = 0; i < synth.getNumVoices(); ++i)
        if (auto* v = dynamic_cast<SynthVoice*> (synth.getVoice (i)))
            v->prepare (sampleRate, samplesPerBlock);

    drums.prepare (sampleRate);
    sampler.prepare (sampleRate);
    sampler.prepareRecorder (sampleRate);
    currentSampleRate = sampleRate;
    dist.prepare (sampleRate);
    delay.prepare (sampleRate, samplesPerBlock);
    reverb.prepare (sampleRate);
    comp.prepare (sampleRate, samplesPerBlock);
    chorus.prepare (sampleRate, samplesPerBlock);
    phaser.prepare (sampleRate, samplesPerBlock);
    eq.prepare (sampleRate, samplesPerBlock);

    synthBuf.setSize (2, samplesPerBlock);
    drumBuf.setSize (2, samplesPerBlock);
    samplerBuf.setSize (2, samplesPerBlock);
}

bool RackForgeProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto& out = layouts.getMainOutputChannelSet();
    const auto& in  = layouts.getMainInputChannelSet();
    const bool outOk = out == juce::AudioChannelSet::stereo() || out == juce::AudioChannelSet::mono();
    const bool inOk  = in.isDisabled() || in == juce::AudioChannelSet::stereo() || in == juce::AudioChannelSet::mono();
    return outOk && inOk;
}

void RackForgeProcessor::mixInto (juce::AudioBuffer<float>& dst, const juce::AudioBuffer<float>& src, float level, float pan, bool mute)
{
    if (mute) return;
    const float angle = (pan + 1.0f) * 0.25f * juce::MathConstants<float>::pi;
    const float gL = level * std::cos (angle) * 1.414f, gR = level * std::sin (angle) * 1.414f;
    const int n = dst.getNumSamples();
    if (dst.getNumChannels() >= 2)
    {
        dst.addFrom (0, 0, src, 0, 0, n, gL);
        dst.addFrom (1, 0, src, 1, 0, n, gR);
    }
    else
        dst.addFrom (0, 0, src, 0, 0, n, level);
}

float RackForgeProcessor::peakOf (const juce::AudioBuffer<float>& b)
{
    float p = 0.0f;
    for (int ch = 0; ch < b.getNumChannels(); ++ch)
        p = juce::jmax (p, b.getMagnitude (ch, 0, b.getNumSamples()));
    return p;
}

void RackForgeProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals noDenormals;
    const int n = buffer.getNumSamples();

    // Entrée audio (si l'hôte l'a activée) -> enregistreur du sampler, avant d'effacer le buffer
    if (getTotalNumInputChannels() > 0)
    {
        inputCopy.setSize (2, n, false, false, true);
        inputCopy.copyFrom (0, 0, buffer, 0, 0, n);
        inputCopy.copyFrom (1, 0, buffer, juce::jmin (1, getTotalNumInputChannels() - 1), 0, n);
        sampler.captureInput (inputCopy, n);
    }
    else
        sampler.inputPeak.store (0.0f);

    buffer.clear();

    // Transport de l'hôte (Cubase)
    double bpm = 120.0, ppq = 0.0;
    bool playing = false, hasPpq = false;
    if (auto* ph = getPlayHead())
    {
        if (auto pos = ph->getPosition())
        {
            if (auto b = pos->getBpm()) bpm = *b;
            if (auto p = pos->getPpqPosition()) { ppq = *p; hasPpq = true; }
            playing = pos->getIsPlaying();
        }
    }
    hostBpm.store ((float) bpm);

    // --- Instruments ---
    synthBuf.setSize (2, n, false, false, true);
    drumBuf.setSize (2, n, false, false, true);
    samplerBuf.setSize (2, n, false, false, true);
    synthBuf.clear();
    drumBuf.clear();
    samplerBuf.clear();

    // Routage MIDI : canal 10 -> boîte à rythmes (notes 36..43), canal 2 -> sampler PADLAB,
    // autres canaux -> synthé VEKTOR
    synthMidi.clear();
    drumMidi.clear();
    samplerMidi.clear();
    for (const auto meta : midi)
    {
        const auto m = meta.getMessage();
        if (m.getChannel() == 10)     drumMidi.addEvent (m, meta.samplePosition);
        else if (m.getChannel() == 2) samplerMidi.addEvent (m, meta.samplePosition);
        else                          synthMidi.addEvent (m, meta.samplePosition);
    }
    // Clic sur un pad dans l'interface
    if (const int pad = uiPadTrigger.exchange (-1); pad >= 0)
    {
        // en mode chromatique, le clic joue le pad sélectionné à sa hauteur d'origine (note 60)
        const int note = (*apvts.getRawParameterValue (ID::padChromatic) > 0.5f) ? 60 : PadSampler::kBaseNote + pad;
        samplerMidi.addEvent (juce::MidiMessage::noteOn (2, note, 1.0f), 0);
        samplerMidi.addEvent (juce::MidiMessage::noteOff (2, note), n - 1);
    }

    // Les instruments retirés du rack restent silencieux ; les effets jouent dans l'ordre du rack
    const auto rack = unpackRack (packedRack.load());
    auto inRack = [&rack] (int m) { return std::find (rack.begin(), rack.end(), m) != rack.end(); };

    if (inRack (modVektor)) synth.renderNextBlock (synthBuf, synthMidi, 0, n);
    if (inRack (modPulse8)) drums.process (drumBuf, drumMidi, playing, hasPpq, ppq, bpm, currentStep);
    else currentStep.store (-1);
    if (inRack (modPadlab)) sampler.process (samplerBuf, samplerMidi);

    // --- Mixeur ---
    mixInto (buffer, synthBuf, *synthLevel, *synthPan, *synthMute > 0.5f);
    mixInto (buffer, drumBuf,  *drumsLevel, *drumsPan, *drumsMute > 0.5f);
    mixInto (buffer, samplerBuf, *samplerLevel, *samplerPan, *samplerMute > 0.5f);

    // --- Chaîne d'effets dans l'ordre du rack ---
    for (int m : rack)
        if (! isInstrument (m)) processEffect (m, buffer, bpm);

    buffer.applyGain (*masterLevel);

    // Vu-mètres (lissés dans l'éditeur)
    meterSynth.store (peakOf (synthBuf) * *synthLevel);
    meterDrums.store (peakOf (drumBuf) * *drumsLevel);
    meterSampler.store (peakOf (samplerBuf) * *samplerLevel);
    meterMaster.store (peakOf (buffer));
}

void RackForgeProcessor::processEffect (int module, juce::AudioBuffer<float>& buffer, double bpm)
{
    switch (module)
    {
        case modGrit:   dist.process (buffer); break;
        case modChorus: chorus.process (buffer); break;
        case modPhaser: phaser.process (buffer); break;
        case modEq:     eq.process (buffer); break;
        case modEcho:   delay.process (buffer, bpm); break;
        case modHall:   reverb.process (buffer); break;
        case modSquash: comp.process (buffer); break;
        default: break;
    }
}

// ------------------------------------------------------------------ Rack
juce::uint64 RackForgeProcessor::packRack (const std::vector<int>& order)
{
    juce::uint64 v = 0;
    int count = 0;
    for (int m : order)
    {
        if (count >= 15 || m < 0 || m >= kNumModules) break;
        v |= (juce::uint64) (m & 0xF) << (4 * count);
        ++count;
    }
    v |= (juce::uint64) count << 60;
    return v;
}

std::vector<int> RackForgeProcessor::unpackRack (juce::uint64 packed)
{
    std::vector<int> order;
    const int count = (int) (packed >> 60);
    for (int i = 0; i < count; ++i) order.push_back ((int) ((packed >> (4 * i)) & 0xF));
    return order;
}

std::vector<int> RackForgeProcessor::getRackOrder() const { return unpackRack (packedRack.load()); }

void RackForgeProcessor::setRackOrder (const std::vector<int>& order)
{
    // dédoublonne et ignore les identifiants invalides
    std::vector<int> clean;
    for (int m : order)
        if (m >= 0 && m < kNumModules && std::find (clean.begin(), clean.end(), m) == clean.end()) clean.push_back (m);
    packedRack.store (packRack (clean));
    ++rackVersion;
}

bool RackForgeProcessor::rackContains (int module) const
{
    auto o = getRackOrder();
    return std::find (o.begin(), o.end(), module) != o.end();
}

void RackForgeProcessor::rackAdd (int module)
{
    if (rackContains (module)) return;
    auto o = getRackOrder();
    if (isInstrument (module))
    {
        auto firstFx = std::find_if (o.begin(), o.end(), [] (int m) { return ! isInstrument (m); });
        o.insert (firstFx, module);
    }
    else o.push_back (module);
    setRackOrder (o);
}

void RackForgeProcessor::rackRemove (int module)
{
    auto o = getRackOrder();
    o.erase (std::remove (o.begin(), o.end(), module), o.end());
    setRackOrder (o);
}

void RackForgeProcessor::rackMove (int module, int delta)
{
    auto o = getRackOrder();
    auto it = std::find (o.begin(), o.end(), module);
    if (it == o.end()) return;
    const int i = (int) (it - o.begin()), j = i + delta;
    if (j < 0 || j >= (int) o.size()) return;
    std::swap (o[(size_t) i], o[(size_t) j]);
    setRackOrder (o);
}

// ------------------------------------------------------------------ État
void RackForgeProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    sampler.saveTo (state);
    juce::StringArray keys;
    for (int m : getRackOrder()) keys.add (kModuleKeys[m]);
    state.getOrCreateChildWithName ("RACK", nullptr).setProperty ("order", keys.joinIntoString (","), nullptr);
    if (auto xml = state.createXml())
        copyXmlToBinary (*xml, destData);
}

void RackForgeProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary (data, sizeInBytes))
        if (xml->hasTagName (apvts.state.getType()))
        {
            auto state = juce::ValueTree::fromXml (*xml);
            apvts.replaceState (state);
            sampler.restoreFrom (state);
            auto rackNode = state.getChildWithName ("RACK");
            if (rackNode.isValid())
            {
                std::vector<int> order;
                for (auto key : juce::StringArray::fromTokens (rackNode.getProperty ("order").toString(), ",", ""))
                    for (int m = 0; m < kNumModules; ++m)
                        if (key == kModuleKeys[m]) order.push_back (m);
                setRackOrder (order);
            }
        }
}

juce::AudioProcessorEditor* RackForgeProcessor::createEditor()
{
    return new RackForgeEditor (*this);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new RackForgeProcessor();
}
