#pragma once
#include <juce_audio_processors/juce_audio_processors.h>

// Identifiants de paramètres du rack RackForge
namespace ID
{
    // --- Synthé "Vektor" ---
    inline const juce::String osc1Wave   = "osc1Wave";
    inline const juce::String osc2Wave   = "osc2Wave";
    inline const juce::String osc2Detune = "osc2Detune";
    inline const juce::String osc2Oct    = "osc2Oct";
    inline const juce::String oscMix     = "oscMix";
    inline const juce::String subLevel   = "subLevel";
    inline const juce::String cutoff     = "cutoff";
    inline const juce::String reso       = "reso";
    inline const juce::String fEnvAmt    = "fEnvAmt";
    inline const juce::String ampA = "ampA", ampD = "ampD", ampS = "ampS", ampR = "ampR";
    inline const juce::String filA = "filA", filD = "filD", filS = "filS", filR = "filR";
    inline const juce::String lfoRate    = "lfoRate";
    inline const juce::String lfoPitch   = "lfoPitch";
    inline const juce::String lfoCutoff  = "lfoCutoff";

    // --- Boîte à rythmes "Pulse-8" ---
    inline const juce::String drumRun    = "drumRun";
    inline const juce::String drumSwing  = "drumSwing";
    inline const juce::String drumPattern = "drumPattern";   // motif A/B/C/D
    inline const juce::String drumChain   = "drumChain";     // enchaîner A->B->C->D
    inline juce::String drumLevel (int t) { return "dLevel" + juce::String (t); }
    inline juce::String drumTune  (int t) { return "dTune"  + juce::String (t); }
    inline juce::String drumDecay (int t) { return "dDecay" + juce::String (t); }
    inline juce::String step (int pat, int t, int s) { return "st" + juce::String (pat) + "_" + juce::String (t) + "_" + juce::String (s); }

    // --- Sampler "PADLAB" ---
    inline const juce::String padChromatic = "padChromatic";   // tous les pads jouent le pad sélectionné en chromatique
    inline const juce::String padSelected  = "padSelected";
    inline juce::String padLevel (int p) { return "pLevel" + juce::String (p); }
    inline juce::String padTune  (int p) { return "pTune"  + juce::String (p); }
    inline juce::String padStart (int p) { return "pStart" + juce::String (p); }
    inline juce::String padEnd   (int p) { return "pEnd"   + juce::String (p); }
    inline juce::String padLoop  (int p) { return "pLoop"  + juce::String (p); }
    inline juce::String padOneShot (int p) { return "pOneShot" + juce::String (p); }
    inline juce::String padAttack (int p) { return "pAtk" + juce::String (p); }
    inline juce::String padRelease (int p) { return "pRel" + juce::String (p); }
    inline juce::String padCutoff (int p) { return "pCut" + juce::String (p); }
    inline juce::String padPan   (int p) { return "pPan"   + juce::String (p); }

    // --- Effets ---
    inline const juce::String distOn = "distOn", distDrive = "distDrive", distTone = "distTone", distMix = "distMix";
    inline const juce::String dlyOn = "dlyOn", dlyTime = "dlyTime", dlyFeedback = "dlyFeedback", dlyMix = "dlyMix";
    inline const juce::String revOn = "revOn", revSize = "revSize", revDamp = "revDamp", revWidth = "revWidth", revMix = "revMix";
    inline const juce::String compOn = "compOn", compThresh = "compThresh", compRatio = "compRatio",
                              compAttack = "compAttack", compRelease = "compRelease", compMakeup = "compMakeup";
    inline const juce::String choOn = "choOn", choRate = "choRate", choDepth = "choDepth", choDelay = "choDelay",
                              choFeedback = "choFeedback", choMix = "choMix";
    inline const juce::String phaOn = "phaOn", phaRate = "phaRate", phaDepth = "phaDepth", phaCentre = "phaCentre",
                              phaFeedback = "phaFeedback", phaMix = "phaMix";
    inline const juce::String eqOn = "eqOn", eqLowGain = "eqLowGain", eqLowFreq = "eqLowFreq",
                              eqMidGain = "eqMidGain", eqMidFreq = "eqMidFreq", eqMidQ = "eqMidQ",
                              eqHighGain = "eqHighGain", eqHighFreq = "eqHighFreq";

    // --- Mixeur ---
    inline const juce::String synthLevel = "synthLevel", synthPan = "synthPan", synthMute = "synthMute";
    inline const juce::String drumsLevel = "drumsLevel", drumsPan = "drumsPan", drumsMute = "drumsMute";
    inline const juce::String samplerLevel = "samplerLevel", samplerPan = "samplerPan", samplerMute = "samplerMute";
    inline const juce::String masterLevel = "masterLevel";
}

inline constexpr int kNumPads       = 16;
inline constexpr int kNumDrumTracks = 8;
inline constexpr int kNumSteps      = 16;
inline constexpr int kNumPatterns   = 4;

// Modules du rack (l'ordre dans le rack est libre ; cet index est l'identifiant stable)
enum ModuleId { modVektor = 0, modPulse8, modPadlab, modGrit, modChorus, modPhaser, modEq, modEcho, modHall, modSquash, kNumModules };
inline const char* const kModuleNames[kNumModules] = { "VEKTOR", "PULSE-8", "PADLAB", "GRIT", "CHORUS", "PHASER", "EQ-3", "ECHO-T", "HALL", "SQUASH" };
inline const char* const kModuleKeys[kNumModules]  = { "vektor", "pulse8", "padlab", "grit", "chorus", "phaser", "eq3", "echo", "hall", "squash" };
inline bool isInstrument (int m) { return m <= modPadlab; }
inline const char* const kDefaultRack = "vektor,pulse8,padlab,grit,echo,hall,squash";

inline const char* const kDrumNames[kNumDrumTracks] =
    { "Kick", "Snare", "HH ferme", "HH ouvert", "Clap", "Tom grave", "Tom aigu", "Rim" };

inline const char* const kWaveNames[] = { "Dent de scie", "Carre", "Triangle", "Sinus" };
inline const char* const kDelayNames[] = { "1/4", "1/8", "1/8 pointee", "1/8 triolet", "1/16", "1/16 triolet" };

inline juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout()
{
    using namespace juce;
    using P = AudioParameterFloat;
    std::vector<std::unique_ptr<RangedAudioParameter>> v;

    auto f = [&] (const String& id, const String& name, float lo, float hi, float def, float skew = 1.0f, const String& unit = {})
    {
        NormalisableRange<float> r (lo, hi, 0.0f, skew);
        v.push_back (std::make_unique<P> (ParameterID { id, 1 }, name, r, def,
                                          AudioParameterFloatAttributes().withLabel (unit)));
    };
    auto b = [&] (const String& id, const String& name, bool def)
    {
        v.push_back (std::make_unique<AudioParameterBool> (ParameterID { id, 1 }, name, def));
    };
    auto c = [&] (const String& id, const String& name, const StringArray& choices, int def)
    {
        v.push_back (std::make_unique<AudioParameterChoice> (ParameterID { id, 1 }, name, choices, def));
    };

    // Synthé
    StringArray waves (kWaveNames, 4);
    c (ID::osc1Wave, "Osc 1 forme", waves, 0);
    c (ID::osc2Wave, "Osc 2 forme", waves, 0);
    f (ID::osc2Detune, "Osc 2 desaccord", -50.0f, 50.0f, 7.0f, 1.0f, "ct");
    c (ID::osc2Oct, "Osc 2 octave", { "-1", "0", "+1" }, 1);
    f (ID::oscMix, "Melange osc", 0.0f, 1.0f, 0.5f);
    f (ID::subLevel, "Sub", 0.0f, 1.0f, 0.2f);
    f (ID::cutoff, "Cutoff", 20.0f, 20000.0f, 1200.0f, 0.3f, "Hz");
    f (ID::reso, "Resonance", 0.0f, 1.0f, 0.2f);
    f (ID::fEnvAmt, "Env -> filtre", -1.0f, 1.0f, 0.5f);
    f (ID::ampA, "Amp A", 0.001f, 5.0f, 0.01f, 0.4f, "s");
    f (ID::ampD, "Amp D", 0.001f, 5.0f, 0.3f, 0.4f, "s");
    f (ID::ampS, "Amp S", 0.0f, 1.0f, 0.8f);
    f (ID::ampR, "Amp R", 0.001f, 8.0f, 0.4f, 0.4f, "s");
    f (ID::filA, "Filtre A", 0.001f, 5.0f, 0.01f, 0.4f, "s");
    f (ID::filD, "Filtre D", 0.001f, 5.0f, 0.5f, 0.4f, "s");
    f (ID::filS, "Filtre S", 0.0f, 1.0f, 0.2f);
    f (ID::filR, "Filtre R", 0.001f, 8.0f, 0.5f, 0.4f, "s");
    f (ID::lfoRate, "LFO vitesse", 0.05f, 20.0f, 4.0f, 0.5f, "Hz");
    f (ID::lfoPitch, "LFO -> hauteur", 0.0f, 1.0f, 0.0f);
    f (ID::lfoCutoff, "LFO -> cutoff", 0.0f, 1.0f, 0.0f);

    // Boîte à rythmes
    b (ID::drumRun, "Sequenceur actif", true);
    f (ID::drumSwing, "Swing", 0.0f, 1.0f, 0.0f);
    c (ID::drumPattern, "Motif", { "A", "B", "C", "D" }, 0);
    b (ID::drumChain, "Enchainer A-B-C-D", false);
    for (int t = 0; t < kNumDrumTracks; ++t)
    {
        String n (kDrumNames[t]);
        f (ID::drumLevel (t), n + " niveau", 0.0f, 1.0f, 0.8f);
        f (ID::drumTune (t),  n + " accord", 0.5f, 2.0f, 1.0f, 0.5f);
        f (ID::drumDecay (t), n + " decay", 0.0f, 1.0f, 0.4f);
    }
    for (int pat = 0; pat < kNumPatterns; ++pat)
        for (int t = 0; t < kNumDrumTracks; ++t)
            for (int s = 0; s < kNumSteps; ++s)
            {
                bool def = false;
                if (pat == 0)   // motif A pré-rempli, B/C/D vides
                {
                    if (t == 0) def = (s % 4 == 0);               // kick sur les temps
                    if (t == 1) def = (s == 4 || s == 12);        // snare 2 et 4
                    if (t == 2) def = (s % 2 == 0);               // charley sur les croches
                }
                b (ID::step (pat, t, s), String ("ABCD").substring (pat, pat + 1) + " " + kDrumNames[t] + " pas " + String (s + 1), def);
            }

    // Sampler
    b (ID::padChromatic, "Mode chromatique", false);
    v.push_back (std::make_unique<AudioParameterInt> (ParameterID { ID::padSelected, 1 }, "Pad selectionne", 0, kNumPads - 1, 0));
    for (int p = 0; p < kNumPads; ++p)
    {
        String n = "Pad " + String (p + 1);
        f (ID::padLevel (p), n + " niveau", 0.0f, 1.0f, 0.8f);
        f (ID::padTune (p),  n + " accord", -24.0f, 24.0f, 0.0f, 1.0f, "st");
        f (ID::padStart (p), n + " depart", 0.0f, 1.0f, 0.0f);
        f (ID::padEnd (p),   n + " fin", 0.0f, 1.0f, 1.0f);
        b (ID::padLoop (p),  n + " boucle", false);
        b (ID::padOneShot (p), n + " one-shot", true);
        f (ID::padAttack (p), n + " attaque", 0.001f, 2.0f, 0.002f, 0.4f, "s");
        f (ID::padRelease (p), n + " relachement", 0.005f, 4.0f, 0.1f, 0.4f, "s");
        f (ID::padCutoff (p), n + " cutoff", 100.0f, 20000.0f, 20000.0f, 0.3f, "Hz");
        f (ID::padPan (p),   n + " pan", -1.0f, 1.0f, 0.0f);
    }

    // Effets
    b (ID::distOn, "Disto active", false);
    f (ID::distDrive, "Drive", 0.0f, 1.0f, 0.3f);
    f (ID::distTone, "Tonalite", 200.0f, 12000.0f, 4000.0f, 0.4f, "Hz");
    f (ID::distMix, "Disto mix", 0.0f, 1.0f, 1.0f);

    b (ID::dlyOn, "Delay actif", false);
    c (ID::dlyTime, "Delay temps", StringArray (kDelayNames, 6), 1);
    f (ID::dlyFeedback, "Feedback", 0.0f, 0.95f, 0.4f);
    f (ID::dlyMix, "Delay mix", 0.0f, 1.0f, 0.3f);

    b (ID::revOn, "Reverb active", true);
    f (ID::revSize, "Taille", 0.0f, 1.0f, 0.6f);
    f (ID::revDamp, "Amortissement", 0.0f, 1.0f, 0.5f);
    f (ID::revWidth, "Largeur", 0.0f, 1.0f, 1.0f);
    f (ID::revMix, "Reverb mix", 0.0f, 1.0f, 0.2f);

    b (ID::compOn, "Comp actif", false);
    f (ID::compThresh, "Seuil", -60.0f, 0.0f, -18.0f, 1.0f, "dB");
    f (ID::compRatio, "Ratio", 1.0f, 20.0f, 4.0f, 0.5f);
    f (ID::compAttack, "Attaque", 0.1f, 200.0f, 10.0f, 0.4f, "ms");
    f (ID::compRelease, "Relachement", 10.0f, 2000.0f, 150.0f, 0.4f, "ms");
    f (ID::compMakeup, "Gain", 0.0f, 24.0f, 0.0f, 1.0f, "dB");

    b (ID::choOn, "Chorus actif", true);
    f (ID::choRate, "Chorus vitesse", 0.05f, 10.0f, 0.8f, 0.5f, "Hz");
    f (ID::choDepth, "Chorus profondeur", 0.0f, 1.0f, 0.35f);
    f (ID::choDelay, "Chorus delai", 1.0f, 50.0f, 12.0f, 0.6f, "ms");
    f (ID::choFeedback, "Chorus feedback", -0.9f, 0.9f, 0.0f);
    f (ID::choMix, "Chorus mix", 0.0f, 1.0f, 0.5f);

    b (ID::phaOn, "Phaser actif", true);
    f (ID::phaRate, "Phaser vitesse", 0.05f, 10.0f, 0.4f, 0.5f, "Hz");
    f (ID::phaDepth, "Phaser profondeur", 0.0f, 1.0f, 0.7f);
    f (ID::phaCentre, "Phaser centre", 100.0f, 8000.0f, 800.0f, 0.4f, "Hz");
    f (ID::phaFeedback, "Phaser feedback", -0.9f, 0.9f, 0.3f);
    f (ID::phaMix, "Phaser mix", 0.0f, 1.0f, 0.5f);

    b (ID::eqOn, "EQ actif", true);
    f (ID::eqLowGain, "Graves gain", -15.0f, 15.0f, 0.0f, 1.0f, "dB");
    f (ID::eqLowFreq, "Graves freq", 40.0f, 500.0f, 120.0f, 0.5f, "Hz");
    f (ID::eqMidGain, "Mediums gain", -15.0f, 15.0f, 0.0f, 1.0f, "dB");
    f (ID::eqMidFreq, "Mediums freq", 200.0f, 6000.0f, 1000.0f, 0.4f, "Hz");
    f (ID::eqMidQ, "Mediums Q", 0.3f, 8.0f, 1.0f, 0.5f);
    f (ID::eqHighGain, "Aigus gain", -15.0f, 15.0f, 0.0f, 1.0f, "dB");
    f (ID::eqHighFreq, "Aigus freq", 2000.0f, 16000.0f, 6000.0f, 0.5f, "Hz");

    // Mixeur
    f (ID::synthLevel, "Synth niveau", 0.0f, 1.0f, 0.8f);
    f (ID::synthPan, "Synth pan", -1.0f, 1.0f, 0.0f);
    b (ID::synthMute, "Synth mute", false);
    f (ID::drumsLevel, "Drums niveau", 0.0f, 1.0f, 0.8f);
    f (ID::drumsPan, "Drums pan", -1.0f, 1.0f, 0.0f);
    b (ID::drumsMute, "Drums mute", false);
    f (ID::samplerLevel, "Sampler niveau", 0.0f, 1.0f, 0.8f);
    f (ID::samplerPan, "Sampler pan", -1.0f, 1.0f, 0.0f);
    b (ID::samplerMute, "Sampler mute", false);
    f (ID::masterLevel, "Master", 0.0f, 1.2f, 0.8f);

    return { v.begin(), v.end() };
}
