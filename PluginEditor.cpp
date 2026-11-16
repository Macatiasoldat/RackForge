#include "PluginEditor.h"

// ============================================================ Look & feel
RackLookAndFeel::RackLookAndFeel()
{
    setColour (juce::Slider::textBoxTextColourId, rf::text);
    setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    setColour (juce::Slider::textBoxBackgroundColourId, juce::Colours::transparentBlack);
    setColour (juce::Label::textColourId, rf::text);
    setColour (juce::ComboBox::backgroundColourId, rf::bg);
    setColour (juce::ComboBox::textColourId, rf::text);
    setColour (juce::ComboBox::outlineColourId, rf::rail);
    setColour (juce::ComboBox::arrowColourId, rf::dim);
    setColour (juce::PopupMenu::backgroundColourId, rf::panelDark);
    setColour (juce::PopupMenu::textColourId, rf::text);
    setColour (juce::PopupMenu::highlightedBackgroundColourId, rf::synthCol);
    setColour (juce::ToggleButton::textColourId, rf::text);
    setColour (juce::ScrollBar::thumbColourId, rf::rail);
}

void RackLookAndFeel::drawRotarySlider (juce::Graphics& g, int x, int y, int w, int h, float pos,
                                        float startAngle, float endAngle, juce::Slider& s)
{
    auto area = juce::Rectangle<float> ((float) x, (float) y, (float) w, (float) h).reduced (4.0f);
    const float size = juce::jmin (area.getWidth(), area.getHeight());
    auto circle = juce::Rectangle<float> (size, size).withCentre (area.getCentre());
    const float r = size * 0.5f;
    const float angle = startAngle + pos * (endAngle - startAngle);
    const auto accent = s.findColour (juce::Slider::rotarySliderFillColourId);

    // arc de fond + arc de valeur
    juce::Path track;
    track.addCentredArc (circle.getCentreX(), circle.getCentreY(), r, r, 0.0f, startAngle, endAngle, true);
    g.setColour (rf::bg);
    g.strokePath (track, juce::PathStrokeType (3.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
    juce::Path val;
    val.addCentredArc (circle.getCentreX(), circle.getCentreY(), r, r, 0.0f, startAngle, angle, true);
    g.setColour (accent);
    g.strokePath (val, juce::PathStrokeType (3.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    // capuchon
    auto cap = circle.reduced (size * 0.18f);
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xff4a4f58), cap.getX(), cap.getY(),
                                             juce::Colour (0xff1c1f24), cap.getRight(), cap.getBottom(), false));
    g.fillEllipse (cap);
    g.setColour (juce::Colour (0xff61666f));
    g.drawEllipse (cap, 1.0f);

    // index
    const float ir = cap.getWidth() * 0.5f;
    juce::Point<float> c = cap.getCentre();
    juce::Point<float> p1 = c.getPointOnCircumference (ir * 0.35f, angle);
    juce::Point<float> p2 = c.getPointOnCircumference (ir * 0.9f, angle);
    g.setColour (juce::Colours::white);
    g.drawLine ({ p1, p2 }, 2.2f);
}

void RackLookAndFeel::drawToggleButton (juce::Graphics& g, juce::ToggleButton& b, bool over, bool)
{
    auto area = b.getLocalBounds().toFloat();
    const bool on = b.getToggleState();
    const bool isStep = b.getProperties().contains ("step");

    if (isStep)
    {
        auto r = area.reduced (2.0f);
        const bool accentBeat = b.getProperties()["beat"];
        g.setColour (on ? rf::led : (accentBeat ? juce::Colour (0xff343941) : juce::Colour (0xff2a2e35)));
        g.fillRoundedRectangle (r, 3.0f);
        if (b.getProperties()["playing"])
        {
            g.setColour (juce::Colours::white.withAlpha (0.9f));
            g.drawRoundedRectangle (r, 3.0f, 1.5f);
        }
        else if (over)
        {
            g.setColour (juce::Colours::white.withAlpha (0.25f));
            g.drawRoundedRectangle (r, 3.0f, 1.0f);
        }
        return;
    }

    // bouton LED avec texte
    auto r = area.reduced (1.0f);
    g.setColour (on ? juce::Colour (0xff3a3e46) : juce::Colour (0xff2a2d33));
    g.fillRoundedRectangle (r, 4.0f);
    g.setColour (over ? juce::Colour (0xff7a808a) : juce::Colour (0xff50555e));
    g.drawRoundedRectangle (r, 4.0f, 1.0f);
    auto ledR = juce::Rectangle<float> (8.0f, 8.0f).withCentre ({ r.getX() + 10.0f, r.getCentreY() });
    g.setColour (on ? rf::led : juce::Colour (0xff3a3020));
    g.fillEllipse (ledR);
    if (on) { g.setColour (rf::led.withAlpha (0.35f)); g.fillEllipse (ledR.expanded (3.0f)); }
    g.setColour (rf::text);
    g.setFont (juce::Font (juce::FontOptions (11.0f, juce::Font::bold)));
    g.drawText (b.getButtonText(), r.withTrimmedLeft (18.0f).toNearestInt(), juce::Justification::centredLeft);
}

void RackLookAndFeel::drawLinearSlider (juce::Graphics& g, int x, int y, int w, int h, float pos, float, float,
                                        juce::Slider::SliderStyle style, juce::Slider& s)
{
    if (style != juce::Slider::LinearVertical) { LookAndFeel_V4::drawLinearSlider (g, x, y, w, h, pos, 0, 0, style, s); return; }
    auto track = juce::Rectangle<float> ((float) x + w * 0.5f - 3.0f, (float) y, 6.0f, (float) h);
    g.setColour (rf::bg);
    g.fillRoundedRectangle (track, 3.0f);
    g.setColour (s.findColour (juce::Slider::rotarySliderFillColourId));
    g.fillRoundedRectangle (track.withTop (pos), 3.0f);
    auto cap = juce::Rectangle<float> (26.0f, 14.0f).withCentre ({ (float) x + w * 0.5f, pos });
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xff5a5f68), cap.getX(), cap.getY(),
                                             juce::Colour (0xff22252a), cap.getX(), cap.getBottom(), false));
    g.fillRoundedRectangle (cap, 3.0f);
    g.setColour (juce::Colours::white.withAlpha (0.7f));
    g.drawLine (cap.getX() + 4.0f, cap.getCentreY(), cap.getRight() - 4.0f, cap.getCentreY(), 1.0f);
}

// ============================================================ DevicePanel
DevicePanel::DevicePanel (juce::AudioProcessorValueTreeState& s, const juce::String& name,
                          const juce::String& sub, juce::Colour col, int height)
    : preferredHeight (height), apvts (s), title (name), subtitle (sub), accent (col)
{
}

Knob& DevicePanel::addKnob (const juce::String& paramId, const juce::String& text)
{
    auto k = std::make_unique<Knob>();
    k->slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    k->slider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 64, 14);
    k->slider.setColour (juce::Slider::rotarySliderFillColourId, accent);
    k->label.setText (text, juce::dontSendNotification);
    k->label.setJustificationType (juce::Justification::centred);
    k->label.setColour (juce::Label::textColourId, rf::dim);
    k->att = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (apvts, paramId, k->slider);
    addAndMakeVisible (k->slider);
    addAndMakeVisible (k->label);
    knobs.push_back (std::move (k));
    return *knobs.back();
}

Choice& DevicePanel::addChoice (const juce::String& paramId, const juce::String& text)
{
    auto c = std::make_unique<Choice>();
    if (auto* p = dynamic_cast<juce::AudioParameterChoice*> (apvts.getParameter (paramId)))
        c->box.addItemList (p->choices, 1);
    c->label.setText (text, juce::dontSendNotification);
    c->label.setJustificationType (juce::Justification::centred);
    c->label.setColour (juce::Label::textColourId, rf::dim);
    c->att = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment> (apvts, paramId, c->box);
    addAndMakeVisible (c->box);
    addAndMakeVisible (c->label);
    choices.push_back (std::move (c));
    return *choices.back();
}

void DevicePanel::setRackButtons (std::function<void (int)> onMove, std::function<void()> onRemove)
{
    auto make = [this] (const juce::String& txt, const juce::String& tip)
    {
        auto b = std::make_unique<juce::TextButton> (txt);
        b->setColour (juce::TextButton::buttonColourId, juce::Colour (0xff33373f));
        b->setColour (juce::TextButton::textColourOffId, rf::dim);
        b->setTooltip (tip);
        addAndMakeVisible (*b);
        return b;
    };
    upBtn = make ("^", "Monter dans le rack");
    downBtn = make ("v", "Descendre dans le rack");
    removeBtn = make ("x", "Retirer du rack");
    upBtn->onClick = [onMove] { onMove (-1); };
    downBtn->onClick = [onMove] { onMove (+1); };
    removeBtn->onClick = [onRemove] { onRemove(); };
    resized();
}

void DevicePanel::addBypass (const juce::String& paramId)
{
    bypass = std::make_unique<juce::ToggleButton> ("ON");
    bypassAtt = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (apvts, paramId, *bypass);
    addAndMakeVisible (*bypass);
}

void DevicePanel::paint (juce::Graphics& g)
{
    auto r = getLocalBounds().toFloat().reduced (2.0f);
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xff2c3038), r.getX(), r.getY(),
                                             rf::panelDark, r.getX(), r.getBottom(), false));
    g.fillRoundedRectangle (r, 6.0f);
    g.setColour (juce::Colour (0xff3a3f48));
    g.drawRoundedRectangle (r, 6.0f, 1.0f);

    // bande de couleur à gauche
    g.setColour (accent);
    g.fillRoundedRectangle (r.getX() + 4.0f, r.getY() + 8.0f, 4.0f, (float) headerH - 12.0f, 2.0f);

    // vis aux quatre coins
    g.setColour (juce::Colour (0xff50555e));
    for (auto p : { r.getTopLeft(), r.getTopRight(), r.getBottomLeft(), r.getBottomRight() })
    {
        auto c = juce::Rectangle<float> (7.0f, 7.0f).withCentre ({ juce::jlimit (r.getX() + 8.0f, r.getRight() - 8.0f, p.x),
                                                                   juce::jlimit (r.getY() + 8.0f, r.getBottom() - 8.0f, p.y) });
        g.fillEllipse (c);
        g.setColour (juce::Colour (0xff1a1c20));
        g.drawLine (c.getX() + 1.5f, c.getCentreY(), c.getRight() - 1.5f, c.getCentreY(), 1.0f);
        g.setColour (juce::Colour (0xff50555e));
    }

    g.setColour (rf::text);
    g.setFont (juce::Font (juce::FontOptions (15.0f, juce::Font::bold)));
    g.drawText (title, 16, 4, 220, headerH - 4, juce::Justification::centredLeft);
    g.setColour (rf::dim);
    g.setFont (juce::Font (juce::FontOptions (11.0f)));
    const int titleW = (int) juce::GlyphArrangement::getStringWidth (juce::Font (juce::FontOptions (15.0f, juce::Font::bold)), title);
    g.drawText (subtitle, 16 + titleW + 10, 4, 500, headerH - 4, juce::Justification::centredLeft);

    g.setColour (juce::Colour (0xff3a3f48));
    g.drawLine (12.0f, (float) headerH, r.getRight() - 10.0f, (float) headerH, 1.0f);
}

void DevicePanel::resized()
{
    int right = getWidth() - 14;
    if (removeBtn)
    {
        removeBtn->setBounds (right - 22, 6, 22, headerH - 12); right -= 26;
        downBtn->setBounds (right - 22, 6, 22, headerH - 12);   right -= 26;
        upBtn->setBounds (right - 22, 6, 22, headerH - 12);     right -= 34;
    }
    if (bypass) bypass->setBounds (right - 60, 5, 60, headerH - 10);

    auto area = contentArea();
    int x = area.getX(), y = area.getY();
    for (auto& c : choices)
    {
        if (x + choiceW > area.getRight()) { x = area.getX(); y += 48; }
        c->label.setBounds (x, y, choiceW, 14);
        c->box.setBounds (x, y + 16, choiceW - 8, 24);
        x += choiceW;
    }
    if (! choices.empty()) { x = area.getX(); y += 48; }
    for (auto& k : knobs)
    {
        if (x + knobW > area.getRight()) { x = area.getX(); y += knobH; }
        k->setBounds (x, y, knobW, knobH);
        x += knobW;
    }
}

// ============================================================ SynthPanel
namespace
{
    struct Preset { const char* name; std::vector<std::pair<juce::String, float>> values; };

    const std::vector<Preset>& presetList()
    {
        static const std::vector<Preset> list = {
            { "Init", { { ID::osc1Wave, 0 }, { ID::osc2Wave, 0 }, { ID::osc2Detune, 7 }, { ID::osc2Oct, 1 }, { ID::oscMix, 0.5f },
                        { ID::subLevel, 0.2f }, { ID::cutoff, 1200 }, { ID::reso, 0.2f }, { ID::fEnvAmt, 0.5f },
                        { ID::ampA, 0.01f }, { ID::ampD, 0.3f }, { ID::ampS, 0.8f }, { ID::ampR, 0.4f },
                        { ID::filA, 0.01f }, { ID::filD, 0.5f }, { ID::filS, 0.2f }, { ID::filR, 0.5f },
                        { ID::lfoRate, 4 }, { ID::lfoPitch, 0 }, { ID::lfoCutoff, 0 } } },
            { "Basse ronde", { { ID::osc1Wave, 1 }, { ID::osc2Wave, 0 }, { ID::osc2Detune, 3 }, { ID::osc2Oct, 0 }, { ID::oscMix, 0.4f },
                        { ID::subLevel, 0.6f }, { ID::cutoff, 300 }, { ID::reso, 0.35f }, { ID::fEnvAmt, 0.6f },
                        { ID::ampA, 0.005f }, { ID::ampD, 0.25f }, { ID::ampS, 0.6f }, { ID::ampR, 0.15f },
                        { ID::filA, 0.005f }, { ID::filD, 0.2f }, { ID::filS, 0.1f }, { ID::filR, 0.2f },
                        { ID::lfoRate, 4 }, { ID::lfoPitch, 0 }, { ID::lfoCutoff, 0 } } },
            { "Nappe large", { { ID::osc1Wave, 0 }, { ID::osc2Wave, 0 }, { ID::osc2Detune, 14 }, { ID::osc2Oct, 1 }, { ID::oscMix, 0.5f },
                        { ID::subLevel, 0.1f }, { ID::cutoff, 900 }, { ID::reso, 0.15f }, { ID::fEnvAmt, 0.3f },
                        { ID::ampA, 0.8f }, { ID::ampD, 1.0f }, { ID::ampS, 0.9f }, { ID::ampR, 2.0f },
                        { ID::filA, 1.5f }, { ID::filD, 2.0f }, { ID::filS, 0.5f }, { ID::filR, 2.0f },
                        { ID::lfoRate, 0.4f }, { ID::lfoPitch, 0.05f }, { ID::lfoCutoff, 0.2f } } },
            { "Lead cuivré", { { ID::osc1Wave, 0 }, { ID::osc2Wave, 1 }, { ID::osc2Detune, -5 }, { ID::osc2Oct, 1 }, { ID::oscMix, 0.35f },
                        { ID::subLevel, 0.0f }, { ID::cutoff, 700 }, { ID::reso, 0.4f }, { ID::fEnvAmt, 0.8f },
                        { ID::ampA, 0.02f }, { ID::ampD, 0.4f }, { ID::ampS, 0.7f }, { ID::ampR, 0.3f },
                        { ID::filA, 0.05f }, { ID::filD, 0.35f }, { ID::filS, 0.3f }, { ID::filR, 0.3f },
                        { ID::lfoRate, 5.5f }, { ID::lfoPitch, 0.08f }, { ID::lfoCutoff, 0 } } },
            { "Pluck", { { ID::osc1Wave, 0 }, { ID::osc2Wave, 2 }, { ID::osc2Detune, 0 }, { ID::osc2Oct, 2 }, { ID::oscMix, 0.3f },
                        { ID::subLevel, 0.0f }, { ID::cutoff, 400 }, { ID::reso, 0.3f }, { ID::fEnvAmt, 0.9f },
                        { ID::ampA, 0.002f }, { ID::ampD, 0.5f }, { ID::ampS, 0.0f }, { ID::ampR, 0.3f },
                        { ID::filA, 0.002f }, { ID::filD, 0.25f }, { ID::filS, 0.0f }, { ID::filR, 0.25f },
                        { ID::lfoRate, 4 }, { ID::lfoPitch, 0 }, { ID::lfoCutoff, 0 } } },
            { "Orgue carré", { { ID::osc1Wave, 1 }, { ID::osc2Wave, 1 }, { ID::osc2Detune, 0 }, { ID::osc2Oct, 2 }, { ID::oscMix, 0.5f },
                        { ID::subLevel, 0.4f }, { ID::cutoff, 5000 }, { ID::reso, 0.0f }, { ID::fEnvAmt, 0.0f },
                        { ID::ampA, 0.005f }, { ID::ampD, 0.1f }, { ID::ampS, 1.0f }, { ID::ampR, 0.05f },
                        { ID::filA, 0.01f }, { ID::filD, 0.1f }, { ID::filS, 1.0f }, { ID::filR, 0.1f },
                        { ID::lfoRate, 6 }, { ID::lfoPitch, 0.03f }, { ID::lfoCutoff, 0 } } },
            { "Wobble", { { ID::osc1Wave, 0 }, { ID::osc2Wave, 1 }, { ID::osc2Detune, 10 }, { ID::osc2Oct, 0 }, { ID::oscMix, 0.5f },
                        { ID::subLevel, 0.5f }, { ID::cutoff, 500 }, { ID::reso, 0.6f }, { ID::fEnvAmt, 0.0f },
                        { ID::ampA, 0.01f }, { ID::ampD, 0.3f }, { ID::ampS, 1.0f }, { ID::ampR, 0.2f },
                        { ID::filA, 0.01f }, { ID::filD, 0.3f }, { ID::filS, 1.0f }, { ID::filR, 0.2f },
                        { ID::lfoRate, 2.0f }, { ID::lfoPitch, 0 }, { ID::lfoCutoff, 0.8f } } },
        };
        return list;
    }
}

SynthPanel::SynthPanel (juce::AudioProcessorValueTreeState& s)
    : DevicePanel (s, "VEKTOR", "synthe polyphonique 8 voix - 2 oscillateurs - filtre - LFO", rf::synthCol, 36 + 48 + 2 * 86 + 14)
{
    addChoice (ID::osc1Wave, "Osc 1");
    addChoice (ID::osc2Wave, "Osc 2");
    addChoice (ID::osc2Oct, "Osc 2 octave");
    addKnob (ID::osc2Detune, "Desaccord");
    addKnob (ID::oscMix, "Melange");
    addKnob (ID::subLevel, "Sub");
    addKnob (ID::cutoff, "Cutoff");
    addKnob (ID::reso, "Resonance");
    addKnob (ID::fEnvAmt, "Env>Filtre");
    addKnob (ID::lfoRate, "LFO vitesse");
    addKnob (ID::lfoPitch, "LFO>Pitch");
    addKnob (ID::lfoCutoff, "LFO>Cutoff");
    addKnob (ID::ampA, "Amp A"); addKnob (ID::ampD, "Amp D"); addKnob (ID::ampS, "Amp S"); addKnob (ID::ampR, "Amp R");
    addKnob (ID::filA, "Filtre A"); addKnob (ID::filD, "Filtre D"); addKnob (ID::filS, "Filtre S"); addKnob (ID::filR, "Filtre R");

    int i = 1;
    for (auto& p : presetList()) presets.addItem (p.name, i++);
    presets.setSelectedId (1, juce::dontSendNotification);
    presets.onChange = [this] { applyPreset (presets.getSelectedId() - 1); };
    presetLabel.setText ("Preset", juce::dontSendNotification);
    presetLabel.setColour (juce::Label::textColourId, rf::dim);
    presetLabel.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (presets);
    addAndMakeVisible (presetLabel);
}

void SynthPanel::applyPreset (int index)
{
    const auto& list = presetList();
    if (index < 0 || index >= (int) list.size()) return;
    for (auto& [id, value] : list[(size_t) index].values)
        if (auto* p = apvts.getParameter (id))
        {
            p->beginChangeGesture();
            p->setValueNotifyingHost (p->convertTo0to1 (value));
            p->endChangeGesture();
        }
}

void SynthPanel::resized()
{
    DevicePanel::resized();
    auto area = contentArea();
    const int x = area.getX() + 3 * choiceW + 10;
    presetLabel.setBounds (x, area.getY(), 150, 14);
    presets.setBounds (x, area.getY() + 16, 150, 24);
}

// ============================================================ DrumPanel
DrumPanel::DrumPanel (juce::AudioProcessorValueTreeState& s, RackForgeProcessor& p)
    : DevicePanel (s, "PULSE-8", "boite a rythmes - 8 sons synthetises - 16 pas synchronises sur l'hote", rf::drumCol,
                   36 + 60 + 8 * 32 + 16),
      proc (p)
{
    runAtt = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (s, ID::drumRun, run);
    addAndMakeVisible (run);
    chainAtt = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (s, ID::drumChain, chain);
    addAndMakeVisible (chain);
    addKnob (ID::drumSwing, "Swing");

    for (int pat = 0; pat < kNumPatterns; ++pat)
    {
        auto& b = patBtn[pat];
        b.setButtonText (juce::String::charToString ((juce::juce_wchar) ('A' + pat)));
        b.setColour (juce::TextButton::buttonColourId, juce::Colour (0xff2e323a));
        b.setColour (juce::TextButton::buttonOnColourId, rf::drumCol);
        b.setColour (juce::TextButton::textColourOffId, rf::text);
        b.setColour (juce::TextButton::textColourOnId, juce::Colours::black);
        b.setTooltip ("Motif " + b.getButtonText() + " : editer et jouer");
        b.onClick = [this, pat]
        {
            if (auto* prm = apvts.getParameter (ID::drumPattern)) prm->setValueNotifyingHost (prm->convertTo0to1 ((float) pat));
            bindPattern (pat);
        };
        addAndMakeVisible (b);
    }
    patMenuBtn.setColour (juce::TextButton::buttonColourId, juce::Colour (0xff3a3e46));
    patMenuBtn.setColour (juce::TextButton::textColourOffId, rf::text);
    patMenuBtn.onClick = [this] { showPatternMenu(); };
    addAndMakeVisible (patMenuBtn);

    for (int t = 0; t < kNumDrumTracks; ++t)
    {
        trackNames[t].setText (kDrumNames[t], juce::dontSendNotification);
        trackNames[t].setColour (juce::Label::textColourId, rf::text);
        trackNames[t].setFont (juce::Font (juce::FontOptions (12.0f, juce::Font::bold)));
        addAndMakeVisible (trackNames[t]);

        auto& lv = addKnob (ID::drumLevel (t), ""); lv.slider.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0); lv.slider.setTooltip ("Niveau");
        auto& tn = addKnob (ID::drumTune (t), "");  tn.slider.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0); tn.slider.setTooltip ("Accord");
        auto& dc = addKnob (ID::drumDecay (t), ""); dc.slider.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0); dc.slider.setTooltip ("Decay");

        for (int st = 0; st < kNumSteps; ++st)
        {
            auto& b = stepBtn[t][st];
            b.getProperties().set ("step", true);
            b.getProperties().set ("beat", (st % 4) == 0);
            b.getProperties().set ("playing", false);
            b.setClickingTogglesState (true);
            addAndMakeVisible (b);
        }
    }
    bindPattern (juce::jlimit (0, kNumPatterns - 1, (int) *s.getRawParameterValue (ID::drumPattern)));
    startTimerHz (30);
}

void DrumPanel::bindPattern (int pat)
{
    editPattern = pat;
    for (int t = 0; t < kNumDrumTracks; ++t)
        for (int st = 0; st < kNumSteps; ++st)
        {
            stepAtt[t][st].reset();
            stepAtt[t][st] = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (apvts, ID::step (pat, t, st), stepBtn[t][st]);
        }
    for (int i = 0; i < kNumPatterns; ++i) patBtn[i].setToggleState (i == pat, juce::dontSendNotification);
    repaint();
}

void DrumPanel::showPatternMenu()
{
    juce::PopupMenu m;
    const juce::String cur = juce::String::charToString ((juce::juce_wchar) ('A' + editPattern));
    m.addSectionHeader ("Motif " + cur);
    for (int i = 0; i < kNumPatterns; ++i)
        if (i != editPattern) m.addItem (10 + i, "Copier " + cur + " vers " + juce::String::charToString ((juce::juce_wchar) ('A' + i)));
    m.addSeparator();
    m.addItem (1, "Effacer le motif " + cur);
    m.showMenuAsync (juce::PopupMenu::Options().withTargetComponent (patMenuBtn), [this] (int r)
    {
        if (r == 0) return;
        for (int t = 0; t < kNumDrumTracks; ++t)
            for (int st = 0; st < kNumSteps; ++st)
            {
                if (r == 1)
                {
                    if (auto* p = apvts.getParameter (ID::step (editPattern, t, st))) p->setValueNotifyingHost (0.0f);
                }
                else
                {
                    const float v = *apvts.getRawParameterValue (ID::step (editPattern, t, st)) > 0.5f ? 1.0f : 0.0f;
                    if (auto* p = apvts.getParameter (ID::step (r - 10, t, st))) p->setValueNotifyingHost (v);
                }
            }
    });
}

void DrumPanel::resized()
{
    DevicePanel::resized();
    auto area = contentArea();
    run.setBounds (area.getX(), area.getY() + 20, 70, 24);
    knobs[0]->setBounds (area.getX() + 90, area.getY() - 4, knobW, knobH - 20);
    int x = area.getX() + 180;
    for (int i = 0; i < kNumPatterns; ++i) { patBtn[i].setBounds (x, area.getY() + 20, 30, 24); x += 32; }
    patMenuBtn.setBounds (x + 6, area.getY() + 20, 70, 24);
    chain.setBounds (x + 86, area.getY() + 20, 170, 24);

    const int rowH = 32, nameW = 84, smallK = 30, stepW = 30;
    int y = area.getY() + 60;
    gridArea = { area.getX() + nameW + 3 * smallK + 12, y, kNumSteps * stepW, kNumDrumTracks * rowH };

    for (int t = 0; t < kNumDrumTracks; ++t)
    {
        trackNames[t].setBounds (area.getX(), y, nameW, rowH);
        for (int k = 0; k < 3; ++k)
        {
            auto& kn = *knobs[(size_t) (1 + t * 3 + k)];
            kn.label.setBounds (0, 0, 0, 0);
            kn.slider.setBounds (area.getX() + nameW + k * smallK, y + 1, smallK, rowH - 2);
        }
        for (int st = 0; st < kNumSteps; ++st)
            stepBtn[t][st].setBounds (gridArea.getX() + st * stepW, y, stepW, rowH);
        y += rowH;
    }
}

void DrumPanel::paint (juce::Graphics& g)
{
    DevicePanel::paint (g);
    auto area = contentArea();
    g.setColour (rf::dim);
    g.setFont (juce::Font (juce::FontOptions (10.0f)));
    const int nameW = 84, smallK = 30;
    g.drawText ("NIV", area.getX() + nameW, area.getY() + 46, smallK, 12, juce::Justification::centred);
    g.drawText ("ACC", area.getX() + nameW + smallK, area.getY() + 46, smallK, 12, juce::Justification::centred);
    g.drawText ("DEC", area.getX() + nameW + 2 * smallK, area.getY() + 46, smallK, 12, juce::Justification::centred);
    for (int st = 0; st < kNumSteps; st += 4)
        g.drawText (juce::String (st / 4 + 1), gridArea.getX() + st * 30, area.getY() + 46, 30, 12, juce::Justification::centred);
    g.drawText ("Canal MIDI 10 : notes 36-43 jouent les pads", gridArea.getRight() - 260, area.getY() + 2, 260, 14,
                juce::Justification::centredRight);
    if (shownPlaying >= 0 && shownPlaying != editPattern)
    {
        g.setColour (rf::led);
        g.drawText ("Lecture : motif " + juce::String::charToString ((juce::juce_wchar) ('A' + shownPlaying)),
                    gridArea.getRight() - 260, area.getY() + 24, 260, 14, juce::Justification::centredRight);
    }
}

void DrumPanel::timerCallback()
{
    const int pat = juce::jlimit (0, kNumPatterns - 1, (int) *apvts.getRawParameterValue (ID::drumPattern));
    if (pat != editPattern) bindPattern (pat);                     // changé par l'hôte / automation
    const int playing = proc.drums.playingPattern.load();
    if (playing != shownPlaying) { shownPlaying = playing; repaint(); }

    const int s = proc.currentStep.load();
    if (s == shownStep) return;
    for (int t = 0; t < kNumDrumTracks; ++t)
    {
        if (shownStep >= 0) { stepBtn[t][shownStep].getProperties().set ("playing", false); stepBtn[t][shownStep].repaint(); }
        if (s >= 0)         { stepBtn[t][s].getProperties().set ("playing", true);          stepBtn[t][s].repaint(); }
    }
    shownStep = s;
}

// ============================================================ SamplerPanel
SamplerPanel::SamplerPanel (juce::AudioProcessorValueTreeState& s, RackForgeProcessor& p)
    : DevicePanel (s, "PADLAB", "sampler 16 pads - vos propres echantillons WAV/AIFF - canal MIDI 2 (notes 48-63)",
                   rf::samplerCol, 36 + 4 * 46 + 100 + 86 + 20),
      proc (p)
{
    for (int i = 0; i < kNumPads; ++i)
    {
        auto& b = pads[i];
        b.setButtonText (juce::String (i + 1));
        b.setColour (juce::TextButton::buttonColourId, juce::Colour (0xff2e323a));
        b.setColour (juce::TextButton::buttonOnColourId, rf::samplerCol.darker (0.4f));
        b.setColour (juce::TextButton::textColourOffId, rf::text);
        b.setColour (juce::TextButton::textColourOnId, juce::Colours::black);
        b.setClickingTogglesState (false);
        b.onClick = [this, i] { selectPad (i); proc.triggerPadFromUI (i); };
        addAndMakeVisible (b);
    }

    // Boutons de réglage du pad sélectionné (re-liés à chaque sélection)
    addKnob (ID::padLevel (0), "Niveau");
    addKnob (ID::padTune (0), "Accord");
    addKnob (ID::padStart (0), "Depart");
    addKnob (ID::padEnd (0), "Fin");
    addKnob (ID::padAttack (0), "Attaque");
    addKnob (ID::padRelease (0), "Relach.");
    addKnob (ID::padCutoff (0), "Cutoff");
    addKnob (ID::padPan (0), "Pan");

    chromAtt = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (s, ID::padChromatic, chromBtn);
    addAndMakeVisible (chromBtn);
    addAndMakeVisible (loopBtn);
    addAndMakeVisible (oneShotBtn);

    loadBtn.onClick = [this] { loadIntoPad (selectedPad); };
    clearBtn.onClick = [this] { proc.sampler.clearPad (selectedPad); refreshPadLabels(); repaint(); };
    recBtn.onClick = [this] { toggleRecord(); };
    sliceBtn.onClick = [this] { showSliceMenu(); };
    for (auto* b : { &loadBtn, &clearBtn, &recBtn, &sliceBtn })
    {
        b->setColour (juce::TextButton::buttonColourId, juce::Colour (0xff3a3e46));
        b->setColour (juce::TextButton::textColourOffId, rf::text);
        addAndMakeVisible (*b);
    }
    recBtn.setColour (juce::TextButton::buttonOnColourId, juce::Colour (0xffd83b3b));
    recBtn.setTooltip ("Enregistre l'entree audio dans le pad selectionne (60 s max). Cliquez a nouveau pour arreter.");
    sliceBtn.setTooltip ("Decoupe le son du pad selectionne en tranches reparties sur les pads suivants");
    sampleName.setColour (juce::Label::textColourId, rf::samplerCol);
    sampleName.setFont (juce::Font (juce::FontOptions (12.0f, juce::Font::bold)));
    addAndMakeVisible (sampleName);

    selectPad ((int) *s.getRawParameterValue (ID::padSelected));
    refreshPadLabels();
    startTimerHz (20);
}

SamplerPanel::~SamplerPanel() = default;

void SamplerPanel::selectPad (int pad)
{
    selectedPad = juce::jlimit (0, kNumPads - 1, pad);
    if (auto* p = apvts.getParameter (ID::padSelected))
        p->setValueNotifyingHost (p->convertTo0to1 ((float) selectedPad));
    for (int i = 0; i < kNumPads; ++i)
        pads[i].setToggleState (i == selectedPad, juce::dontSendNotification);
    rebindKnobs();
    refreshPadLabels();
    repaint();
}

void SamplerPanel::rebindKnobs()
{
    const juce::String ids[8] = { ID::padLevel (selectedPad), ID::padTune (selectedPad), ID::padStart (selectedPad),
                                  ID::padEnd (selectedPad), ID::padAttack (selectedPad), ID::padRelease (selectedPad),
                                  ID::padCutoff (selectedPad), ID::padPan (selectedPad) };
    for (int i = 0; i < 8; ++i)
    {
        knobs[(size_t) i]->att.reset();
        knobs[(size_t) i]->att = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (apvts, ids[i], knobs[(size_t) i]->slider);
    }
    loopAtt.reset();
    oneShotAtt.reset();
    loopAtt = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (apvts, ID::padLoop (selectedPad), loopBtn);
    oneShotAtt = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (apvts, ID::padOneShot (selectedPad), oneShotBtn);
}

void SamplerPanel::refreshPadLabels()
{
    for (int i = 0; i < kNumPads; ++i)
    {
        auto smp = proc.sampler.getSample (i);
        pads[i].setButtonText (smp ? juce::String (i + 1) + "\n" + smp->name.substring (0, 10) : juce::String (i + 1));
        pads[i].setTooltip (smp ? smp->file.getFullPathName() : "Pad vide - cliquez sur Charger ou glissez un fichier");
    }
    auto sel = proc.sampler.getSample (selectedPad);
    sampleName.setText ("Pad " + juce::String (selectedPad + 1) + " : " + (sel ? sel->name : juce::String ("(vide)")),
                        juce::dontSendNotification);
}

void SamplerPanel::loadIntoPad (int pad)
{
    chooser = std::make_unique<juce::FileChooser> ("Choisir un echantillon pour le pad " + juce::String (pad + 1),
                                                   juce::File::getSpecialLocation (juce::File::userMusicDirectory),
                                                   "*.wav;*.aif;*.aiff;*.flac;*.ogg;*.mp3");
    chooser->launchAsync (juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
                          [this, pad] (const juce::FileChooser& fc)
                          {
                              auto f = fc.getResult();
                              if (f.existsAsFile())
                              {
                                  proc.sampler.loadFile (pad, f);
                                  refreshPadLabels();
                                  repaint();
                              }
                          });
}

void SamplerPanel::toggleRecord()
{
    if (proc.sampler.isRecording())
    {
        proc.sampler.stopRecording (proc.currentSampleRate);
        recBtn.setToggleState (false, juce::dontSendNotification);
        recBtn.setButtonText ("REC");
        refreshPadLabels();
        repaint();
        return;
    }
    if (! proc.hasAudioInput())
    {
        juce::AlertWindow::showMessageBoxAsync (juce::MessageBoxIconType::InfoIcon, "Pas d'entree audio",
            "Aucune entree audio n'est connectee a RackForge.\n\n"
            "- Version Standalone : choisissez l'entree de votre interface audio dans Options > Audio/MIDI.\n"
            "- Cubase : activez l'entree side-chain du plugin et routez-y une piste audio (si votre version le permet), "
            "ou exportez l'audio en WAV puis glissez-le sur un pad.");
        return;
    }
    proc.sampler.startRecording (selectedPad);
    recBtn.setToggleState (true, juce::dontSendNotification);
}

void SamplerPanel::showSliceMenu()
{
    if (proc.sampler.getSample (selectedPad) == nullptr) return;
    juce::PopupMenu m;
    m.addSectionHeader ("Decouper le pad " + juce::String (selectedPad + 1) + " vers les pads suivants");
    m.addItem (1, "Automatique (transitoires, sensibilite normale)");
    m.addItem (2, "Automatique (sensibilite haute)");
    m.addItem (3, "Automatique (sensibilite basse)");
    m.addSeparator();
    m.addItem (4, "4 parts egales");
    m.addItem (8, "8 parts egales");
    m.addItem (16, "16 parts egales (max 15 pads)");
    m.showMenuAsync (juce::PopupMenu::Options().withTargetComponent (sliceBtn), [this] (int r)
    {
        if (r == 0) return;
        int made = 0;
        if (r == 1) made = proc.sampler.sliceToPads (selectedPad, 0, 0.5f);
        else if (r == 2) made = proc.sampler.sliceToPads (selectedPad, 0, 0.9f);
        else if (r == 3) made = proc.sampler.sliceToPads (selectedPad, 0, 0.15f);
        else made = proc.sampler.sliceToPads (selectedPad, r);
        // les tranches sont en one-shot sans boucle
        for (int i = 1; i <= made; ++i)
        {
            if (auto* p = apvts.getParameter (ID::padOneShot (selectedPad + i))) p->setValueNotifyingHost (1.0f);
            if (auto* p = apvts.getParameter (ID::padLoop (selectedPad + i)))    p->setValueNotifyingHost (0.0f);
            if (auto* p = apvts.getParameter (ID::padStart (selectedPad + i)))   p->setValueNotifyingHost (0.0f);
            if (auto* p = apvts.getParameter (ID::padEnd (selectedPad + i)))     p->setValueNotifyingHost (1.0f);
        }
        refreshPadLabels();
        sampleName.setText (made > 0 ? juce::String (made) + " tranche(s) placee(s) sur les pads " + juce::String (selectedPad + 2)
                                           + " a " + juce::String (selectedPad + 1 + made)
                                     : juce::String ("Aucune tranche trouvee"), juce::dontSendNotification);
        repaint();
    });
}

bool SamplerPanel::isInterestedInFileDrag (const juce::StringArray& files)
{
    for (auto& f : files)
        if (juce::File (f).hasFileExtension ("wav;aif;aiff;flac;ogg;mp3")) return true;
    return false;
}

void SamplerPanel::filesDropped (const juce::StringArray& files, int x, int y)
{
    // Déposé sur un pad : ce pad ; sinon : le pad sélectionné (puis les suivants si plusieurs fichiers)
    int target = selectedPad;
    for (int i = 0; i < kNumPads; ++i)
        if (pads[i].getBounds().contains (x, y)) { target = i; break; }
    const int first = target;
    for (auto& f : files)
    {
        if (target >= kNumPads) break;
        if (proc.sampler.loadFile (target, juce::File (f))) ++target;
    }
    selectPad (first);
}

void SamplerPanel::resized()
{
    auto area = contentArea();
    const int padSize = 46;
    padArea = { area.getX(), area.getY(), 4 * padSize, 4 * padSize };
    for (int i = 0; i < kNumPads; ++i)
    {
        const int row = 3 - i / 4, col = i % 4;    // pad 1 en bas à gauche, comme sur une machine à pads
        pads[i].setBounds (padArea.getX() + col * padSize, padArea.getY() + row * padSize, padSize - 3, padSize - 3);
    }

    auto right = area.withTrimmedLeft (padArea.getWidth() + 16);
    sampleName.setBounds (right.getX(), right.getY(), 300, 18);
    loadBtn.setBounds (right.getRight() - 190, right.getY(), 90, 22);
    clearBtn.setBounds (right.getRight() - 90, right.getY(), 70, 22);
    recBtn.setBounds (right.getRight() - 420, right.getY(), 56, 22);
    inputMeterRect = { right.getRight() - 358, right.getY() + 7, 60, 8 };
    sliceBtn.setBounds (right.getRight() - 280, right.getY(), 70, 22);
    waveArea = { right.getX(), right.getY() + 24, right.getWidth(), 100 };

    int y = waveArea.getBottom() + 8, x = right.getX();
    oneShotBtn.setBounds (x, y, 90, 22); loopBtn.setBounds (x + 96, y, 80, 22); chromBtn.setBounds (x + 182, y, 120, 22);

    y += 28;
    for (auto& k : knobs)
    {
        if (x + knobW > right.getRight()) { x = right.getX(); y += knobH; }
        k->setBounds (x, y, knobW, knobH);
        x += knobW;
    }
}

void SamplerPanel::paint (juce::Graphics& g)
{
    DevicePanel::paint (g);

    // Forme d'onde du pad sélectionné
    auto w = waveArea.toFloat();
    g.setColour (rf::bg);
    g.fillRoundedRectangle (w, 4.0f);
    auto smp = proc.sampler.getSample (selectedPad);
    if (smp != nullptr && smp->numSamples() > 0)
    {
        const int n = smp->numSamples();
        const int cols = (int) w.getWidth() - 4;
        if (smp.get() != cachedSample || (int) peaks.size() != cols * 2)
        {
            cachedSample = smp.get();
            peaks.assign ((size_t) cols * 2, 0.0f);
            const float* d = smp->buffer.getReadPointer (0);
            for (int c = 0; c < cols; ++c)
            {
                const int i0 = (int) ((juce::int64) c * n / cols), i1 = juce::jmax (i0 + 1, (int) ((juce::int64) (c + 1) * n / cols));
                float lo = 0.0f, hi = 0.0f;
                for (int i = i0; i < i1 && i < n; ++i) { lo = juce::jmin (lo, d[i]); hi = juce::jmax (hi, d[i]); }
                peaks[(size_t) c * 2] = lo; peaks[(size_t) c * 2 + 1] = hi;
            }
        }
        juce::Path path;
        const float midY = w.getCentreY(), half = w.getHeight() * 0.45f;
        for (int c = 0; c < cols; ++c)
        {
            const float lo = peaks[(size_t) c * 2], hi = peaks[(size_t) c * 2 + 1];
            path.addRectangle (w.getX() + 2.0f + (float) c, midY - hi * half, 1.0f, juce::jmax (1.0f, (hi - lo) * half));
        }
        g.setColour (rf::samplerCol.withAlpha (0.85f));
        g.fillPath (path);

        // Marqueurs départ / fin
        auto& slot = proc.sampler.slot (selectedPad);
        const float xs = w.getX() + 2.0f + *slot.start * (float) cols;
        const float xe = w.getX() + 2.0f + *slot.end * (float) cols;
        g.setColour (juce::Colours::black.withAlpha (0.55f));
        g.fillRect (w.getX(), w.getY(), xs - w.getX(), w.getHeight());
        g.fillRect (xe, w.getY(), w.getRight() - xe, w.getHeight());
        g.setColour (juce::Colours::white);
        g.drawLine (xs, w.getY(), xs, w.getBottom(), 1.5f);
        g.drawLine (xe, w.getY(), xe, w.getBottom(), 1.5f);

        g.setColour (rf::dim);
        g.setFont (juce::Font (juce::FontOptions (10.0f)));
        g.drawText (juce::String (n / smp->sourceSampleRate, 2) + " s  -  " + juce::String ((int) smp->sourceSampleRate) + " Hz",
                    waveArea.reduced (6, 2), juce::Justification::bottomRight);
    }
    else
    {
        g.setColour (rf::dim);
        g.setFont (juce::Font (juce::FontOptions (12.0f)));
        g.drawText ("Glissez un fichier WAV / AIFF ici ou cliquez sur Charger...", waveArea, juce::Justification::centred);
    }

    // Vu-mètre d'entrée (enregistreur)
    {
        auto r = inputMeterRect.toFloat();
        g.setColour (rf::bg);
        g.fillRoundedRectangle (r, 2.0f);
        const float db = juce::Decibels::gainToDecibels (inputMeter, -60.0f);
        const float norm = juce::jlimit (0.0f, 1.0f, (db + 60.0f) / 60.0f);
        g.setColour (proc.hasAudioInput() ? (norm > 0.95f ? juce::Colour (0xffe8463c) : rf::fxCol) : juce::Colour (0xff3a3e46));
        g.fillRoundedRectangle (r.withWidth (r.getWidth() * norm), 2.0f);
        if (! proc.hasAudioInput())
        {
            g.setColour (rf::dim);
            g.setFont (juce::Font (juce::FontOptions (9.0f)));
            g.drawText ("pas d'entree", inputMeterRect.translated (0, 10), juce::Justification::centred);
        }
    }

    // Pad en train de jouer
    if (flashPad >= 0)
    {
        g.setColour (rf::samplerCol.withAlpha (0.5f));
        g.drawRoundedRectangle (pads[flashPad].getBounds().toFloat().expanded (1.0f), 4.0f, 2.0f);
    }
}

void SamplerPanel::timerCallback()
{
    const int t = proc.sampler.lastTriggered.exchange (-1);
    if (t >= 0) { flashPad = t; flashCount = 3; repaint (padArea.expanded (3)); }
    else if (flashCount > 0 && --flashCount == 0) { flashPad = -1; repaint (padArea.expanded (3)); }

    // suit le pad sélectionné si changé par l'hôte (automation)
    const int sel = (int) *apvts.getRawParameterValue (ID::padSelected);
    if (sel != selectedPad) selectPad (sel);

    inputMeter = juce::jmax (proc.sampler.inputPeak.load(), inputMeter * 0.8f);
    if (proc.sampler.isRecording())
        recBtn.setButtonText ("STOP " + juce::String (proc.sampler.recordedSeconds (proc.currentSampleRate), 1) + "s");
    else if (recBtn.getToggleState())   // arrêt automatique (60 s atteintes)
        toggleRecord();
    repaint (inputMeterRect.expanded (2, 12));
    repaint (waveArea);
}

// ============================================================ MixerPanel
MixerPanel::MixerPanel (juce::AudioProcessorValueTreeState& s, RackForgeProcessor& p)
    : DevicePanel (s, "MIX-4", "table de mixage - niveaux, panoramiques, vu-metres", rf::mixCol, 36 + 190 + 16), proc (p)
{
    const juce::String names[4]  = { "VEKTOR", "PULSE-8", "PADLAB", "MASTER" };
    const juce::String levels[4] = { ID::synthLevel, ID::drumsLevel, ID::samplerLevel, ID::masterLevel };
    const juce::String pans[3]   = { ID::synthPan, ID::drumsPan, ID::samplerPan };
    const juce::String mutes[3]  = { ID::synthMute, ID::drumsMute, ID::samplerMute };
    const juce::Colour cols[4]   = { rf::synthCol, rf::drumCol, rf::samplerCol, rf::mixCol };

    for (int i = 0; i < 4; ++i)
    {
        auto& st = strips[i];
        st.name.setText (names[i], juce::dontSendNotification);
        st.name.setJustificationType (juce::Justification::centred);
        st.name.setColour (juce::Label::textColourId, cols[i]);
        st.name.setFont (juce::Font (juce::FontOptions (12.0f, juce::Font::bold)));
        addAndMakeVisible (st.name);

        st.fader.setSliderStyle (juce::Slider::LinearVertical);
        st.fader.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 50, 14);
        st.fader.setColour (juce::Slider::rotarySliderFillColourId, cols[i]);
        st.faderAtt = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (s, levels[i], st.fader);
        addAndMakeVisible (st.fader);

        if (i < 3)
        {
            st.pan = std::make_unique<Knob>();
            st.pan->slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
            st.pan->slider.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
            st.pan->slider.setColour (juce::Slider::rotarySliderFillColourId, cols[i]);
            st.pan->label.setText ("Pan", juce::dontSendNotification);
            st.pan->label.setJustificationType (juce::Justification::centred);
            st.pan->label.setColour (juce::Label::textColourId, rf::dim);
            st.pan->att = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (s, pans[i], st.pan->slider);
            addAndMakeVisible (st.pan->slider);
            addAndMakeVisible (st.pan->label);

            st.mute = std::make_unique<juce::ToggleButton> ("MUTE");
            st.muteAtt = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (s, mutes[i], *st.mute);
            addAndMakeVisible (*st.mute);
        }
    }
    startTimerHz (25);
}

void MixerPanel::resized()
{
    auto area = contentArea();
    const int stripW = 150;
    for (int i = 0; i < 4; ++i)
    {
        auto& st = strips[i];
        auto r = juce::Rectangle<int> (area.getX() + i * (stripW + 20), area.getY(), stripW, area.getHeight());
        st.name.setBounds (r.removeFromTop (18));
        auto faderArea = r.removeFromLeft (60);
        st.fader.setBounds (faderArea.withTrimmedBottom (4));
        st.meterRect = r.removeFromLeft (14).reduced (0, 20);
        r.removeFromLeft (10);
        if (st.pan)
        {
            st.pan->setBounds (r.getX(), r.getY(), 60, 66);
            st.mute->setBounds (r.getX(), r.getY() + 80, 64, 24);
        }
    }
}

void MixerPanel::paint (juce::Graphics& g)
{
    DevicePanel::paint (g);
    for (auto& st : strips)
    {
        auto r = st.meterRect.toFloat();
        g.setColour (rf::bg);
        g.fillRoundedRectangle (r, 2.0f);
        const float db = juce::Decibels::gainToDecibels (st.meter, -60.0f);
        const float norm = juce::jlimit (0.0f, 1.0f, (db + 60.0f) / 60.0f);
        auto fill = r.withTop (r.getBottom() - r.getHeight() * norm);
        g.setGradientFill (juce::ColourGradient (juce::Colour (0xffe8463c), 0, r.getY(),
                                                 juce::Colour (0xff4ba37a), 0, r.getBottom(), false));
        g.fillRoundedRectangle (fill, 2.0f);
    }
}

void MixerPanel::timerCallback()
{
    const float v[4] = { proc.meterSynth.load(), proc.meterDrums.load(), proc.meterSampler.load(), proc.meterMaster.load() };
    for (int i = 0; i < 4; ++i)
        strips[i].meter = juce::jmax (v[i], strips[i].meter * 0.82f);
    repaint();
}

// ============================================================ Editor
RackForgeEditor::RackForgeEditor (RackForgeProcessor& p)
    : AudioProcessorEditor (&p), proc (p)
{
    setLookAndFeel (&lnf);

    mixer = std::make_unique<MixerPanel> (proc.apvts, proc);
    rack.addAndMakeVisible (*mixer);
    addBtn.setColour (juce::TextButton::buttonColourId, juce::Colour (0xff3a3e46));
    addBtn.setColour (juce::TextButton::textColourOffId, rf::text);
    addBtn.onClick = [this] { showAddMenu(); };
    rack.addAndMakeVisible (addBtn);

    viewport.setViewedComponent (&rack, false);
    viewport.setScrollBarsShown (true, false);
    addAndMakeVisible (viewport);

    bpmLabel.setColour (juce::Label::textColourId, rf::dim);
    bpmLabel.setJustificationType (juce::Justification::centredRight);
    addAndMakeVisible (bpmLabel);

    rebuildRack();

    setResizable (true, true);
    setResizeLimits (760, 480, 1400, 1600);
    setSize (900, 720);
    startTimerHz (8);
}

std::unique_ptr<DevicePanel> RackForgeEditor::makePanel (int module)
{
    auto& s = proc.apvts;
    std::unique_ptr<DevicePanel> pnl;
    auto fx = [&] (const juce::String& sub, const juce::String& onId)
    {
        pnl = std::make_unique<DevicePanel> (s, kModuleNames[module], sub, rf::fxCol, 36 + 86 + 14);
        pnl->addBypass (onId);
    };

    switch (module)
    {
        case modVektor: pnl = std::make_unique<SynthPanel> (s); break;
        case modPulse8: pnl = std::make_unique<DrumPanel> (s, proc); break;
        case modPadlab: pnl = std::make_unique<SamplerPanel> (s, proc); break;
        case modGrit:
            fx ("distorsion a saturation douce", ID::distOn);
            pnl->addKnob (ID::distDrive, "Drive"); pnl->addKnob (ID::distTone, "Tonalite"); pnl->addKnob (ID::distMix, "Mix");
            break;
        case modChorus:
            fx ("chorus stereo", ID::choOn);
            pnl->addKnob (ID::choRate, "Vitesse"); pnl->addKnob (ID::choDepth, "Profondeur"); pnl->addKnob (ID::choDelay, "Delai");
            pnl->addKnob (ID::choFeedback, "Feedback"); pnl->addKnob (ID::choMix, "Mix");
            break;
        case modPhaser:
            fx ("phaser 6 etages", ID::phaOn);
            pnl->addKnob (ID::phaRate, "Vitesse"); pnl->addKnob (ID::phaDepth, "Profondeur"); pnl->addKnob (ID::phaCentre, "Centre");
            pnl->addKnob (ID::phaFeedback, "Feedback"); pnl->addKnob (ID::phaMix, "Mix");
            break;
        case modEq:
            fx ("egaliseur 3 bandes - graves / mediums / aigus", ID::eqOn);
            pnl->addKnob (ID::eqLowGain, "Graves"); pnl->addKnob (ID::eqLowFreq, "Freq G");
            pnl->addKnob (ID::eqMidGain, "Mediums"); pnl->addKnob (ID::eqMidFreq, "Freq M"); pnl->addKnob (ID::eqMidQ, "Q");
            pnl->addKnob (ID::eqHighGain, "Aigus"); pnl->addKnob (ID::eqHighFreq, "Freq A");
            break;
        case modEcho:
            fx ("delay synchronise au tempo", ID::dlyOn);
            pnl->preferredHeight += 48;
            pnl->addChoice (ID::dlyTime, "Temps"); pnl->addKnob (ID::dlyFeedback, "Feedback"); pnl->addKnob (ID::dlyMix, "Mix");
            break;
        case modHall:
            fx ("reverberation", ID::revOn);
            pnl->addKnob (ID::revSize, "Taille"); pnl->addKnob (ID::revDamp, "Amort."); pnl->addKnob (ID::revWidth, "Largeur"); pnl->addKnob (ID::revMix, "Mix");
            break;
        case modSquash:
            fx ("compresseur", ID::compOn);
            pnl->addKnob (ID::compThresh, "Seuil"); pnl->addKnob (ID::compRatio, "Ratio"); pnl->addKnob (ID::compAttack, "Attaque");
            pnl->addKnob (ID::compRelease, "Relach."); pnl->addKnob (ID::compMakeup, "Gain");
            break;
        default: return nullptr;
    }
    pnl->moduleId = module;
    pnl->setRackButtons ([this, module] (int delta) { proc.rackMove (module, delta); },
                         [this, module] { proc.rackRemove (module); });
    return pnl;
}

void RackForgeEditor::rebuildRack()
{
    shownRackVersion = proc.rackVersion.load();
    panels.clear();
    for (int m : proc.getRackOrder())
        if (auto p = makePanel (m))
        {
            rack.addAndMakeVisible (*p);
            panels.push_back (std::move (p));
        }
    resized();
}

void RackForgeEditor::showAddMenu()
{
    juce::PopupMenu m, inst, fx;
    for (int i = 0; i < kNumModules; ++i)
        if (! proc.rackContains (i))
            (isInstrument (i) ? inst : fx).addItem (100 + i, kModuleNames[i]);
    if (inst.getNumItems() > 0) m.addSubMenu ("Instruments", inst);
    if (fx.getNumItems() > 0) m.addSubMenu ("Effets", fx);
    if (m.getNumItems() == 0) m.addItem (1, "Tous les modules sont deja dans le rack", false);
    m.showMenuAsync (juce::PopupMenu::Options().withTargetComponent (addBtn), [this] (int r)
    {
        if (r >= 100) proc.rackAdd (r - 100);
    });
}

RackForgeEditor::~RackForgeEditor()
{
    setLookAndFeel (nullptr);
}

void RackForgeEditor::paint (juce::Graphics& g)
{
    g.fillAll (rf::bg);
    auto header = getLocalBounds().removeFromTop (44).toFloat();
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xff30343c), 0, 0, rf::bg, 0, header.getBottom(), false));
    g.fillRect (header);
    g.setColour (rf::text);
    g.setFont (juce::Font (juce::FontOptions (22.0f, juce::Font::bold)));
    g.drawText ("RACKFORGE", 16, 0, 200, 44, juce::Justification::centredLeft);
    g.setColour (rf::dim);
    g.setFont (juce::Font (juce::FontOptions (11.0f)));
    g.drawText ("studio a rack virtuel  -  v0.4", 170, 0, 300, 44, juce::Justification::centredLeft);

    // rails du rack
    g.setColour (rf::rail);
    g.fillRect (0, 44, 10, getHeight() - 44);
    g.fillRect (getWidth() - 22, 44, 10, getHeight() - 44);
}

void RackForgeEditor::resized()
{
    bpmLabel.setBounds (getWidth() - 180, 0, 160, 44);
    viewport.setBounds (12, 48, getWidth() - 24, getHeight() - 52);
    const int w = viewport.getWidth() - 12;
    int y = 0;
    for (auto& pnl : panels)
    {
        pnl->setBounds (0, y, w, pnl->preferredHeight);
        y += pnl->preferredHeight + 4;
    }
    addBtn.setBounds (14, y + 4, 200, 26);
    y += 38;
    mixer->setBounds (0, y, w, mixer->preferredHeight);
    y += mixer->preferredHeight + 4;
    rack.setSize (w, y);
}

void RackForgeEditor::timerCallback()
{
    bpmLabel.setText (juce::String (proc.hostBpm.load(), 1) + " BPM", juce::dontSendNotification);
    if (proc.rackVersion.load() != shownRackVersion) rebuildRack();
}
