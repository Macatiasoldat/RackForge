#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_formats/juce_audio_formats.h>
#include "Params.h"

// Un échantillon chargé en mémoire (partagé entre le thread audio et l'interface via shared_ptr)
struct SampleData
{
    juce::AudioBuffer<float> buffer;   // toujours stéréo
    double sourceSampleRate = 44100.0;
    juce::String name;
    juce::File file;
    int numSamples() const { return buffer.getNumSamples(); }
};

using SamplePtr = std::shared_ptr<const SampleData>;

// Un pad : paramètres + échantillon
struct PadSlot
{
    std::atomic<float>* level = nullptr; std::atomic<float>* tune = nullptr;
    std::atomic<float>* start = nullptr; std::atomic<float>* end = nullptr;
    std::atomic<float>* loop = nullptr;  std::atomic<float>* oneShot = nullptr;
    std::atomic<float>* attack = nullptr; std::atomic<float>* release = nullptr;
    std::atomic<float>* cutoff = nullptr; std::atomic<float>* pan = nullptr;

    // Pointeur vers l'échantillon : l'UI écrit, l'audio lit (std::atomic_load sur shared_ptr)
    std::shared_ptr<const SampleData> sample;
    std::mutex sampleMutex;

    SamplePtr get()
    {
        std::lock_guard<std::mutex> lg (sampleMutex);
        return sample;
    }
    void set (SamplePtr s)
    {
        std::lock_guard<std::mutex> lg (sampleMutex);
        sample = std::move (s);
    }
};

class SamplerVoice
{
public:
    void prepare (double sr_) { sr = sr_; active = false; }

    void start (int padIndex, PadSlot& slot, SamplePtr s, float velocity, float semitoneOffset)
    {
        if (s == nullptr || s->numSamples() < 2) { active = false; return; }
        pad = padIndex; sample = std::move (s);
        const int n = sample->numSamples();
        startPos = juce::jlimit (0.0, (double) n - 2, (double) *slot.start * n);
        endPos   = juce::jlimit (startPos + 1.0, (double) n - 1, (double) *slot.end * n);
        pos = startPos;
        looping = *slot.loop > 0.5f;
        oneShot = *slot.oneShot > 0.5f;
        rate = (sample->sourceSampleRate / sr) * std::pow (2.0, (*slot.tune + semitoneOffset) / 12.0);
        gain = velocity * *slot.level;
        const float angle = (*slot.pan + 1.0f) * 0.25f * juce::MathConstants<float>::pi;
        gL = std::cos (angle) * 1.414f; gR = std::sin (angle) * 1.414f;
        env.setSampleRate (sr);
        env.setParameters ({ *slot.attack, 0.0f, 1.0f, *slot.release });
        env.noteOn();
        const float fc = juce::jlimit (20.0f, (float) (sr * 0.45), slot.cutoff->load());
        lpCoef = (fc >= 19000.0f) ? 1.0f : juce::jlimit (0.0f, 1.0f, juce::MathConstants<float>::twoPi * fc / (float) sr);
        lp[0] = lp[1] = 0.0f;
        active = true; released = false;
    }

    void release()
    {
        if (! active || oneShot) return;
        env.noteOff(); released = true;
    }
    void kill() { active = false; }
    bool isActive() const { return active; }
    int  padIndex() const { return pad; }
    bool wasReleased() const { return released; }

    void render (juce::AudioBuffer<float>& out, int num)
    {
        if (! active) return;
        const auto& b = sample->buffer;
        const float* L = b.getReadPointer (0);
        const float* R = b.getReadPointer (juce::jmin (1, b.getNumChannels() - 1));
        float* oL = out.getWritePointer (0);
        float* oR = out.getWritePointer (juce::jmin (1, out.getNumChannels() - 1));

        for (int i = 0; i < num; ++i)
        {
            if (pos >= endPos)
            {
                if (looping) pos = startPos + std::fmod (pos - startPos, juce::jmax (1.0, endPos - startPos));
                else { active = false; return; }
            }
            const int i0 = (int) pos;
            const float frac = (float) (pos - i0);
            const int i1 = juce::jmin (i0 + 1, sample->numSamples() - 1);
            float l = L[i0] + (L[i1] - L[i0]) * frac;
            float r = R[i0] + (R[i1] - R[i0]) * frac;
            if (lpCoef < 1.0f)
            {
                lp[0] += lpCoef * (l - lp[0]); l = lp[0];
                lp[1] += lpCoef * (r - lp[1]); r = lp[1];
            }
            const float e = env.getNextSample() * gain;
            oL[i] += l * e * gL;
            oR[i] += r * e * gR;
            pos += rate;
            if (! env.isActive()) { active = false; return; }
        }
    }

private:
    double sr = 44100.0, pos = 0.0, startPos = 0.0, endPos = 0.0, rate = 1.0;
    float gain = 1.0f, gL = 1.0f, gR = 1.0f, lpCoef = 1.0f, lp[2] { 0, 0 };
    bool active = false, looping = false, oneShot = true, released = false;
    int pad = -1;
    SamplePtr sample;
    juce::ADSR env;
};

// Sampler à 16 pads "PADLAB"
class PadSampler
{
public:
    static constexpr int kBaseNote = 48;   // C3 : pad 1 = note 48, pad 16 = note 63
    static constexpr int kNumVoices = 24;

    PadSampler()
    {
        formats.registerBasicFormats();
    }

    void attach (juce::AudioProcessorValueTreeState& s)
    {
        chromatic = s.getRawParameterValue (ID::padChromatic);
        selected  = s.getRawParameterValue (ID::padSelected);
        for (int p = 0; p < kNumPads; ++p)
        {
            auto& sl = pads[p];
            sl.level = s.getRawParameterValue (ID::padLevel (p));
            sl.tune = s.getRawParameterValue (ID::padTune (p));
            sl.start = s.getRawParameterValue (ID::padStart (p));
            sl.end = s.getRawParameterValue (ID::padEnd (p));
            sl.loop = s.getRawParameterValue (ID::padLoop (p));
            sl.oneShot = s.getRawParameterValue (ID::padOneShot (p));
            sl.attack = s.getRawParameterValue (ID::padAttack (p));
            sl.release = s.getRawParameterValue (ID::padRelease (p));
            sl.cutoff = s.getRawParameterValue (ID::padCutoff (p));
            sl.pan = s.getRawParameterValue (ID::padPan (p));
        }
    }

    void prepare (double sr)
    {
        sampleRate = sr;
        for (auto& v : voices) v.prepare (sr);
    }

    // Charge un fichier audio dans un pad (appelé depuis le thread d'interface)
    bool loadFile (int pad, const juce::File& file)
    {
        if (pad < 0 || pad >= kNumPads) return false;
        std::unique_ptr<juce::AudioFormatReader> reader (formats.createReaderFor (file));
        if (reader == nullptr) return false;
        const juce::int64 maxLen = (juce::int64) (reader->sampleRate * 60.0);           // max 60 s
        const int n = (int) (reader->lengthInSamples < maxLen ? reader->lengthInSamples : maxLen);
        if (n < 2) return false;
        auto data = std::make_shared<SampleData>();
        data->buffer.setSize (2, n);
        reader->read (&data->buffer, 0, n, 0, true, true);
        if (reader->numChannels == 1) data->buffer.copyFrom (1, 0, data->buffer, 0, 0, n);
        data->sourceSampleRate = reader->sampleRate;
        data->name = file.getFileNameWithoutExtension();
        data->file = file;
        pads[pad].set (data);
        return true;
    }

    void clearPad (int pad)
    {
        if (pad >= 0 && pad < kNumPads) pads[pad].set (nullptr);
    }

    SamplePtr getSample (int pad) { return (pad >= 0 && pad < kNumPads) ? pads[pad].get() : nullptr; }
    PadSlot& slot (int pad) { return pads[juce::jlimit (0, kNumPads - 1, pad)]; }

    void trigger (int pad, float velocity, float semitoneOffset = 0.0f)
    {
        if (pad < 0 || pad >= kNumPads) return;
        auto s = pads[pad].get();
        if (s == nullptr) return;
        lastTriggered.store (pad);
        // One-shot : relancer le même pad coupe l'instance précédente (sauf chromatique)
        SamplerVoice* v = nullptr;
        for (auto& vo : voices) if (! vo.isActive()) { v = &vo; break; }
        if (v == nullptr) { v = &voices[0]; for (auto& vo : voices) if (vo.wasReleased()) { v = &vo; break; } }
        v->start (pad, pads[pad], s, velocity, semitoneOffset);
    }

    void releasePad (int pad, float semitoneOffset = 0.0f)
    {
        juce::ignoreUnused (semitoneOffset);
        for (auto& v : voices) if (v.isActive() && v.padIndex() == pad) v.release();
    }

    void handleMidi (const juce::MidiMessage& m)
    {
        const bool chrom = *chromatic > 0.5f;
        if (m.isNoteOn())
        {
            if (chrom)
            {
                const int sel = (int) *selected;
                trigger (sel, m.getFloatVelocity(), (float) (m.getNoteNumber() - 60));
            }
            else
                trigger (m.getNoteNumber() - kBaseNote, m.getFloatVelocity());
        }
        else if (m.isNoteOff())
        {
            if (chrom) releasePad ((int) *selected);
            else releasePad (m.getNoteNumber() - kBaseNote);
        }
        else if (m.isAllNotesOff() || m.isAllSoundOff())
            for (auto& v : voices) v.kill();
    }

    void process (juce::AudioBuffer<float>& out, const juce::MidiBuffer& midi)
    {
        const int num = out.getNumSamples();
        int last = 0;
        for (const auto meta : midi)
        {
            const int at = juce::jlimit (0, num, meta.samplePosition);
            if (at > last) { renderRange (out, last, at - last); last = at; }
            handleMidi (meta.getMessage());
        }
        if (last < num) renderRange (out, last, num - last);
    }

    // Sauvegarde / restauration des chemins de fichiers
    void saveTo (juce::ValueTree& state)
    {
        auto node = state.getOrCreateChildWithName ("PADLAB", nullptr);
        node.removeAllChildren (nullptr);
        for (int p = 0; p < kNumPads; ++p)
            if (auto s = pads[p].get())
            {
                // Enregistrements et tranches n'ont pas de fichier : on les écrit en WAV dans Documents/RackForge/Samples
                if (! s->file.existsAsFile())
                {
                    auto dir = juce::File::getSpecialLocation (juce::File::userDocumentsDirectory).getChildFile ("RackForge").getChildFile ("Samples");
                    dir.createDirectory();
                    auto f = dir.getChildFile (juce::File::createLegalFileName (s->name) + "_" + juce::String::toHexString ((int) juce::Random::getSystemRandom().nextInt()) + ".wav");
                    juce::WavAudioFormat fmt;
                    if (auto out = std::unique_ptr<juce::FileOutputStream> (f.createOutputStream()))
                    {
                        std::unique_ptr<juce::AudioFormatWriter> w (fmt.createWriterFor (out.get(), s->sourceSampleRate, 2, 24, {}, 0));
                        if (w != nullptr)
                        {
                            out.release();
                            w->writeFromAudioSampleBuffer (s->buffer, 0, s->numSamples());
                            w.reset();
                            auto copy = std::make_shared<SampleData> (*s);
                            copy->file = f;
                            pads[p].set (copy);
                            s = copy;
                        }
                    }
                }
                juce::ValueTree padNode ("pad");
                padNode.setProperty ("index", p, nullptr);
                padNode.setProperty ("path", s->file.getFullPathName(), nullptr);
                node.addChild (padNode, -1, nullptr);
            }
    }

    void restoreFrom (const juce::ValueTree& state)
    {
        auto node = state.getChildWithName ("PADLAB");
        if (! node.isValid()) return;
        for (int i = 0; i < node.getNumChildren(); ++i)
        {
            auto c = node.getChild (i);
            juce::File f (c.getProperty ("path").toString());
            if (f.existsAsFile()) loadFile ((int) c.getProperty ("index"), f);
        }
    }

    std::atomic<int> lastTriggered { -1 };

    // ------------------------------------------------------------ Enregistrement (entrée audio -> pad)
    static constexpr double kMaxRecordSeconds = 60.0;

    void prepareRecorder (double sr)
    {
        recBuffer.setSize (2, (int) (sr * kMaxRecordSeconds));
        recBuffer.clear();
        recLength.store (0);
        recording.store (false);
    }

    // Démarre / arrête depuis l'interface. À l'arrêt, l'audio capturé devient l'échantillon du pad.
    void startRecording (int pad)
    {
        recPad = juce::jlimit (0, kNumPads - 1, pad);
        recLength.store (0);
        recording.store (true);
    }

    void stopRecording (double sr)
    {
        recording.store (false);
        int n = recLength.load();
        // rogne le silence de début (< -50 dB)
        int first = 0;
        while (first < n && std::abs (recBuffer.getSample (0, first)) < 0.003f && std::abs (recBuffer.getSample (1, first)) < 0.003f) ++first;
        first = juce::jmax (0, first - (int) (sr * 0.005));
        n -= first;
        if (n < 64) return;
        auto data = std::make_shared<SampleData>();
        data->buffer.setSize (2, n);
        data->buffer.copyFrom (0, 0, recBuffer, 0, first, n);
        data->buffer.copyFrom (1, 0, recBuffer, 1, first, n);
        // normalisation à -1 dB
        const float peak = juce::jmax (data->buffer.getMagnitude (0, 0, n), data->buffer.getMagnitude (1, 0, n));
        if (peak > 0.0001f) data->buffer.applyGain (0.89f / peak);
        data->sourceSampleRate = sr;
        data->name = "REC " + juce::Time::getCurrentTime().formatted ("%H:%M:%S");
        pads[recPad].set (data);
    }

    bool isRecording() const { return recording.load(); }
    float recordedSeconds (double sr) const { return (float) (recLength.load() / sr); }

    // Appelé par le thread audio avec le bus d'entrée
    void captureInput (const juce::AudioBuffer<float>& in, int numSamples)
    {
        float pk = 0.0f;
        for (int ch = 0; ch < in.getNumChannels(); ++ch) pk = juce::jmax (pk, in.getMagnitude (ch, 0, numSamples));
        inputPeak.store (pk);
        if (! recording.load()) return;
        const int pos = recLength.load();
        const int room = recBuffer.getNumSamples() - pos;
        const int n = juce::jmin (numSamples, room);
        if (n <= 0) { recording.store (false); return; }
        recBuffer.copyFrom (0, pos, in, 0, 0, n);
        recBuffer.copyFrom (1, pos, in, juce::jmin (1, in.getNumChannels() - 1), 0, n);
        recLength.store (pos + n);
    }

    std::atomic<float> inputPeak { 0.0f };

    // ------------------------------------------------------------ Découpage en slices
    // Découpe l'échantillon du pad source selon ses transitoires (ou en parts égales si equalParts > 0)
    // et répartit les tranches sur les pads suivants. Retourne le nombre de tranches créées.
    int sliceToPads (int sourcePad, int equalParts = 0, float sensitivity = 0.5f)
    {
        auto src = pads[juce::jlimit (0, kNumPads - 1, sourcePad)].get();
        if (src == nullptr) return 0;
        const int n = src->numSamples();
        const double sr = src->sourceSampleRate;
        std::vector<int> cuts;

        if (equalParts > 0)
        {
            for (int i = 0; i < equalParts; ++i) cuts.push_back ((int) ((juce::int64) i * n / equalParts));
        }
        else
        {
            // Détection de transitoires sur l'enveloppe d'énergie (fenêtres de 5 ms)
            const int win = juce::jmax (32, (int) (sr * 0.005));
            const int frames = n / win;
            std::vector<float> env ((size_t) frames, 0.0f);
            for (int f = 0; f < frames; ++f)
            {
                float e = 0.0f;
                for (int i = 0; i < win; ++i) { const float s = src->buffer.getSample (0, f * win + i); e += s * s; }
                env[(size_t) f] = std::sqrt (e / win);
            }
            float maxE = 0.0f; for (float e : env) maxE = juce::jmax (maxE, e);
            if (maxE <= 0.0f) return 0;
            const float thresh = (0.35f - sensitivity * 0.3f) * maxE;          // sensibilité haute = seuil bas
            const int minGap = (int) (sr * 0.06) / win;                          // 60 ms minimum entre deux coups
            float slow = 0.0f; int lastCut = -minGap;
            for (int f = 1; f < frames; ++f)
            {
                slow = slow * 0.9f + env[(size_t) f - 1] * 0.1f;
                const float rise = env[(size_t) f] - slow;
                if (rise > thresh && env[(size_t) f] > thresh && f - lastCut >= minGap)
                {
                    cuts.push_back (juce::jmax (0, f * win - win / 2));
                    lastCut = f;
                }
            }
            if (cuts.empty() || cuts.front() > win * 4) cuts.insert (cuts.begin(), 0);
        }

        const int maxSlices = kNumPads - sourcePad - 1;
        if (maxSlices <= 0) return 0;
        if ((int) cuts.size() > maxSlices) cuts.resize ((size_t) maxSlices);

        int made = 0;
        for (size_t i = 0; i < cuts.size(); ++i)
        {
            const int a = cuts[i];
            const int b = (i + 1 < cuts.size()) ? cuts[i + 1] : n;
            const int len = b - a;
            if (len < 32) continue;
            auto d = std::make_shared<SampleData>();
            d->buffer.setSize (2, len);
            d->buffer.copyFrom (0, 0, src->buffer, 0, a, len);
            d->buffer.copyFrom (1, 0, src->buffer, 1, a, len);
            // fondu de 2 ms aux extrémités pour éviter les clics
            const int fade = juce::jmin (len / 4, (int) (sr * 0.002));
            d->buffer.applyGainRamp (0, fade, 0.0f, 1.0f);
            d->buffer.applyGainRamp (len - fade, fade, 1.0f, 0.0f);
            d->sourceSampleRate = sr;
            d->name = src->name + " #" + juce::String (made + 1);
            d->file = juce::File();   // tranche en mémoire : non rechargeable depuis un chemin
            pads[sourcePad + 1 + made].set (d);
            ++made;
        }
        return made;
    }

private:
    juce::AudioBuffer<float> recBuffer;
    std::atomic<int> recLength { 0 };
    std::atomic<bool> recording { false };
    int recPad = 0;

    void renderRange (juce::AudioBuffer<float>& out, int start, int num)
    {
        if (num <= 0) return;
        juce::AudioBuffer<float> sub (out.getArrayOfWritePointers(), out.getNumChannels(), start, num);
        for (auto& v : voices) v.render (sub, num);
    }

    double sampleRate = 44100.0;
    juce::AudioFormatManager formats;
    PadSlot pads[kNumPads];
    SamplerVoice voices[kNumVoices];
    std::atomic<float>* chromatic = nullptr;
    std::atomic<float>* selected = nullptr;
};
