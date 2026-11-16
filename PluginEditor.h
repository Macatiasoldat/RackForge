#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include "PluginProcessor.h"

namespace rf
{
    const juce::Colour bg        (0xff16181c);
    const juce::Colour rail      (0xff2b2f36);
    const juce::Colour panelDark (0xff23262c);
    const juce::Colour text      (0xffd8dbe0);
    const juce::Colour dim       (0xff8a909a);
    const juce::Colour synthCol  (0xff3b7dd8);
    const juce::Colour drumCol   (0xffd8743b);
    const juce::Colour samplerCol (0xffd8b43b);
    const juce::Colour fxCol     (0xff4ba37a);
    const juce::Colour mixCol    (0xff8e5fd8);
    const juce::Colour led       (0xffffb347);
}

// Look & feel "rack métal" du studio
class RackLookAndFeel : public juce::LookAndFeel_V4
{
public:
    RackLookAndFeel();
    void drawRotarySlider (juce::Graphics&, int x, int y, int w, int h, float pos,
                           float startAngle, float endAngle, juce::Slider&) override;
    void drawToggleButton (juce::Graphics&, juce::ToggleButton&, bool, bool) override;
    void drawLinearSlider (juce::Graphics&, int x, int y, int w, int h, float pos, float, float,
                           juce::Slider::SliderStyle, juce::Slider&) override;
    juce::Font getLabelFont (juce::Label&) override { return juce::Font (juce::FontOptions (11.0f)); }
};

// Un bouton rotatif avec son étiquette et sa liaison au paramètre
struct Knob
{
    juce::Slider slider;
    juce::Label label;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> att;
    void setBounds (int x, int y, int w, int h)
    {
        label.setBounds (x, y, w, 14);
        slider.setBounds (x, y + 14, w, h - 14);
    }
};

struct Choice
{
    juce::ComboBox box;
    juce::Label label;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> att;
};

// Panneau de rack générique : barre de titre, vis, LED de bypass, grille de boutons
class DevicePanel : public juce::Component
{
public:
    DevicePanel (juce::AudioProcessorValueTreeState& s, const juce::String& name,
                 const juce::String& subtitle, juce::Colour accent, int height);

    void paint (juce::Graphics&) override;
    void resized() override;

    Knob&   addKnob (const juce::String& paramId, const juce::String& text);
    Choice& addChoice (const juce::String& paramId, const juce::String& text);
    void    addBypass (const juce::String& paramId);

    int preferredHeight;
    int moduleId = -1;

    // Boutons de rack (monter / descendre / retirer) : fournis par l'éditeur
    void setRackButtons (std::function<void (int)> onMove, std::function<void()> onRemove);

protected:
    std::unique_ptr<juce::TextButton> upBtn, downBtn, removeBtn;
    juce::AudioProcessorValueTreeState& apvts;
    juce::String title, subtitle;
    juce::Colour accent;
    std::vector<std::unique_ptr<Knob>> knobs;
    std::vector<std::unique_ptr<Choice>> choices;
    std::unique_ptr<juce::ToggleButton> bypass;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> bypassAtt;

    static constexpr int headerH = 30, knobW = 72, knobH = 86, choiceW = 110;
    juce::Rectangle<int> contentArea() const { return getLocalBounds().reduced (14, 0).withTrimmedTop (headerH + 6).withTrimmedBottom (8); }
};

// Synthé : presets + rangées de boutons
class SynthPanel : public DevicePanel
{
public:
    explicit SynthPanel (juce::AudioProcessorValueTreeState& s);
    void resized() override;
private:
    void applyPreset (int index);
    juce::ComboBox presets;
    juce::Label presetLabel;
};

// Boîte à rythmes : 8 pistes x 16 pas
class DrumPanel : public DevicePanel, private juce::Timer
{
public:
    DrumPanel (juce::AudioProcessorValueTreeState& s, RackForgeProcessor& p);
    void resized() override;
    void paint (juce::Graphics&) override;
private:
    void timerCallback() override;
    void bindPattern (int pat);
    void showPatternMenu();
    RackForgeProcessor& proc;
    juce::ToggleButton run { "RUN" }, chain { "ENCHAINER A-B-C-D" };
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> runAtt, chainAtt;
    juce::TextButton patBtn[kNumPatterns], patMenuBtn { "Copier..." };
    juce::Label trackNames[kNumDrumTracks];
    juce::ToggleButton stepBtn[kNumDrumTracks][kNumSteps];
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> stepAtt[kNumDrumTracks][kNumSteps];
    int shownStep = -1, editPattern = 0, shownPlaying = -1;
    juce::Rectangle<int> gridArea;
};

// Sampler : 16 pads, forme d'onde, réglages du pad sélectionné
class SamplerPanel : public DevicePanel, private juce::Timer, public juce::FileDragAndDropTarget
{
public:
    SamplerPanel (juce::AudioProcessorValueTreeState& s, RackForgeProcessor& p);
    ~SamplerPanel() override;
    void resized() override;
    void paint (juce::Graphics&) override;
    bool isInterestedInFileDrag (const juce::StringArray& files) override;
    void filesDropped (const juce::StringArray& files, int x, int y) override;

private:
    void timerCallback() override;
    void selectPad (int pad);
    void rebindKnobs();
    void loadIntoPad (int pad);
    void refreshPadLabels();

    RackForgeProcessor& proc;
    int selectedPad = 0;
    juce::TextButton pads[kNumPads];
    juce::TextButton loadBtn { "Charger..." }, clearBtn { "Vider" }, recBtn { "REC" }, sliceBtn { "Slices" };
    juce::Rectangle<int> inputMeterRect;
    float inputMeter = 0.0f;
    void toggleRecord();
    void showSliceMenu();
    juce::ToggleButton loopBtn { "BOUCLE" }, oneShotBtn { "ONE-SHOT" }, chromBtn { "CHROMATIQUE" };
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> loopAtt, oneShotAtt, chromAtt;
    juce::Label sampleName;
    juce::Rectangle<int> waveArea, padArea;
    std::unique_ptr<juce::FileChooser> chooser;
    int flashPad = -1, flashCount = 0;
    const SampleData* cachedSample = nullptr;
    std::vector<float> peaks;
};

// Mixeur : faders, pans, mutes, vu-mètres
class MixerPanel : public DevicePanel, private juce::Timer
{
public:
    MixerPanel (juce::AudioProcessorValueTreeState& s, RackForgeProcessor& p);
    void resized() override;
    void paint (juce::Graphics&) override;
private:
    void timerCallback() override;
    struct Strip
    {
        juce::Label name;
        juce::Slider fader;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> faderAtt;
        std::unique_ptr<Knob> pan;
        std::unique_ptr<juce::ToggleButton> mute;
        std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> muteAtt;
        float meter = 0.0f;
        juce::Rectangle<int> meterRect;
    };
    RackForgeProcessor& proc;
    Strip strips[4];
};

class RackForgeEditor : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    explicit RackForgeEditor (RackForgeProcessor&);
    ~RackForgeEditor() override;
    void paint (juce::Graphics&) override;
    void resized() override;
private:
    void timerCallback() override;
    void rebuildRack();
    std::unique_ptr<DevicePanel> makePanel (int module);
    void showAddMenu();
    RackForgeProcessor& proc;
    RackLookAndFeel lnf;
    juce::Viewport viewport;
    juce::Component rack;
    std::vector<std::unique_ptr<DevicePanel>> panels;
    std::unique_ptr<MixerPanel> mixer;
    juce::Label bpmLabel;
    juce::TextButton addBtn { "+ Ajouter un module" };
    int shownRackVersion = -1;
    juce::TooltipWindow tooltips { nullptr, 600 };
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RackForgeEditor)
};
