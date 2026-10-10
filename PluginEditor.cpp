#include "PluginEditor.h"

DonswayCleanTakeAudioProcessorEditor::DonswayCleanTakeAudioProcessorEditor (DonswayCleanTakeAudioProcessor& p)
    : AudioProcessorEditor (&p), processor (p)
{
    setSize (520, 380);
    title.setText ("DONS'WAY CLEANTAKE", juce::dontSendNotification);
    title.setFont (juce::Font (juce::FontOptions (25.0f, juce::Font::bold)));
    title.setColour (juce::Label::textColourId, juce::Colour (0xffe5c477));
    title.setJustificationType (juce::Justification::centredLeft);
    addAndMakeVisible (title);

    subtitle.setText ("REAL-TIME VOCAL CLEANER  •  VST3", juce::dontSendNotification);
    subtitle.setFont (juce::Font (juce::FontOptions (11.0f)));
    subtitle.setColour (juce::Label::textColourId, juce::Colour (0xffb7bbc0));
    addAndMakeVisible (subtitle);

    styleSlider (reduction); styleSlider (rumble); styleSlider (harshness); styleSlider (gate);
    addAndMakeVisible (reduction); addAndMakeVisible (rumble); addAndMakeVisible (harshness); addAndMakeVisible (gate);
    reduction.setName ("Noise Reduction"); rumble.setName ("Rumble Cut (Hz)");
    harshness.setName ("Harshness"); gate.setName ("Gate Strength");

    reductionAttachment = std::make_unique<SliderAttachment>(processor.parameters, "clean", reduction);
    rumbleAttachment = std::make_unique<SliderAttachment>(processor.parameters, "rumble", rumble);
    harshAttachment = std::make_unique<SliderAttachment>(processor.parameters, "harsh", harshness);
    gateAttachment = std::make_unique<SliderAttachment>(processor.parameters, "gate", gate);

    for (auto* b : { &denoiseButton, &harshButton, &gateButton })
    {
        b->setColour (juce::ToggleButton::textColourId, juce::Colour (0xfff2f2ee));
        b->setColour (juce::ToggleButton::tickColourId, juce::Colour (0xffe5c477));
        addAndMakeVisible (*b);
    }
    denoiseAttachment = std::make_unique<ButtonAttachment>(processor.parameters, "denoiseOn", denoiseButton);
    harshButtonAttachment = std::make_unique<ButtonAttachment>(processor.parameters, "harshOn", harshButton);
    gateButtonAttachment = std::make_unique<ButtonAttachment>(processor.parameters, "gateOn", gateButton);
}

void DonswayCleanTakeAudioProcessorEditor::styleSlider (juce::Slider& s)
{
    s.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    s.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 76, 20);
    s.setColour (juce::Slider::rotarySliderFillColourId, juce::Colour (0xffe5c477));
    s.setColour (juce::Slider::rotarySliderOutlineColourId, juce::Colour (0xff42464b));
    s.setColour (juce::Slider::thumbColourId, juce::Colour (0xfff2f2ee));
    s.setColour (juce::Slider::textBoxTextColourId, juce::Colour (0xfff2f2ee));
    s.setColour (juce::Slider::textBoxBackgroundColourId, juce::Colour (0xff24272a));
    s.setColour (juce::Slider::textBoxOutlineColourId, juce::Colour (0xff42464b));
}

void DonswayCleanTakeAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xff151719));
    g.setColour (juce::Colour (0xff2d3135));
    g.drawRoundedRectangle (getLocalBounds().toFloat().reduced (8.0f), 12.0f, 1.0f);
    g.setColour (juce::Colour (0xffe5c477));
    g.fillRoundedRectangle (14.0f, 68.0f, (float)getWidth() - 28.0f, 2.0f, 1.0f);
    g.setColour (juce::Colour (0xfff2f2ee));
    g.setFont (juce::Font (juce::FontOptions (12.0f, juce::Font::bold)));
    g.drawText ("NOISE REDUCTION", 22, 88, 140, 18, juce::Justification::centred);
    g.drawText ("RUMBLE", 186, 88, 140, 18, juce::Justification::centred);
    g.drawText ("HARSHNESS", 350, 88, 140, 18, juce::Justification::centred);
    g.drawText ("GATE STRENGTH", 22, 232, 140, 18, juce::Justification::centred);
}

void DonswayCleanTakeAudioProcessorEditor::resized()
{
    title.setBounds (20, 14, 330, 34);
    subtitle.setBounds (22, 45, 300, 18);
    reduction.setBounds (20, 106, 145, 116);
    rumble.setBounds (184, 106, 145, 116);
    harshness.setBounds (348, 106, 145, 116);
    gate.setBounds (20, 250, 145, 112);
    denoiseButton.setBounds (185, 263, 145, 26);
    harshButton.setBounds (185, 294, 145, 26);
    gateButton.setBounds (185, 325, 145, 26);
}
