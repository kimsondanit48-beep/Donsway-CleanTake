#pragma once
#include <JuceHeader.h>

class DonswayCleanTakeAudioProcessor final : public juce::AudioProcessor
{
public:
    DonswayCleanTakeAudioProcessor();
    ~DonswayCleanTakeAudioProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }
    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}
    void getStateInformation (juce::MemoryBlock&) override;
    void setStateInformation (const void*, int) override;

    juce::AudioProcessorValueTreeState parameters;

private:
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    void updateFilters();
    float processOneSample (float x, int channel);

    double currentSampleRate = 44100.0;
    juce::AudioBuffer<float> noiseBuffer;
    std::vector<float> noiseEnvelope;
    std::vector<float> gateEnvelope;
    std::vector<float> harshEnvelope;
    std::vector<float> previousInput;
    std::vector<float> previousHighpassInput;
    std::vector<float> previousHighpassOutput;
    std::vector<float> previousLowpassInput;
    std::vector<float> previousLowpassOutput;
    std::vector<float> previousHarshInput;
    std::vector<float> previousHarshOutput;
    std::vector<float> previousPresenceInput;
    std::vector<float> previousPresenceOutput;
    float highpassA = 0.0f, lowpassA = 0.0f, harshA = 0.0f, presenceA = 0.0f;
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DonswayCleanTakeAudioProcessor)
};
