#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

class DonswayCleanTakeAudioProcessorEditor final : public juce::AudioProcessorEditor
{
public:
    explicit DonswayCleanTakeAudioProcessorEditor (DonswayCleanTakeAudioProcessor&);
    ~DonswayCleanTakeAudioProcessorEditor() override = default;
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    DonswayCleanTakeAudioProcessor& processor;
    juce::Slider reduction, rumble, harshness, gate;
    juce::ToggleButton denoiseButton {"ADAPTIVE CLEAN"}, harshButton {"HARSHNESS"}, gateButton {"GENTLE GATE"};
    juce::Label title, subtitle;
    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ButtonAttachment = juce::AudioProcessorValueTreeState::ButtonAttachment;
    std::unique_ptr<SliderAttachment> reductionAttachment, rumbleAttachment, harshAttachment, gateAttachment;
    std::unique_ptr<ButtonAttachment> denoiseAttachment, harshButtonAttachment, gateButtonAttachment;
    void styleSlider (juce::Slider&);
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DonswayCleanTakeAudioProcessorEditor)
};
