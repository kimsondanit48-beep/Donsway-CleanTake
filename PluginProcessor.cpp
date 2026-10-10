#include "PluginProcessor.h"
#include "PluginEditor.h"

DonswayCleanTakeAudioProcessor::DonswayCleanTakeAudioProcessor()
    : AudioProcessor (BusesProperties()
        .withInput ("Input", juce::AudioChannelSet::stereo(), true)
        .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      parameters (*this, nullptr, "PARAMETERS", createParameterLayout())
{
}

juce::AudioProcessorValueTreeState::ParameterLayout DonswayCleanTakeAudioProcessor::createParameterLayout()
{
    using P = juce::AudioProcessorValueTreeState;
    P::ParameterLayout layout;
    layout.add (std::make_unique<juce::AudioParameterFloat>("clean", "Noise Reduction", 0.0f, 100.0f, 35.0f));
    layout.add (std::make_unique<juce::AudioParameterFloat>("rumble", "Rumble Cut", 20.0f, 180.0f, 75.0f));
    layout.add (std::make_unique<juce::AudioParameterFloat>("harsh", "Harshness", 0.0f, 100.0f, 40.0f));
    layout.add (std::make_unique<juce::AudioParameterFloat>("gate", "Gate", 0.0f, 100.0f, 25.0f));
    layout.add (std::make_unique<juce::AudioParameterBool>("gateOn", "Gate Enabled", true));
    layout.add (std::make_unique<juce::AudioParameterBool>("denoiseOn", "Denoise Enabled", true));
    layout.add (std::make_unique<juce::AudioParameterBool>("harshOn", "Harshness Enabled", true));
    return layout;
}

bool DonswayCleanTakeAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto in = layouts.getMainInputChannelSet();
    const auto out = layouts.getMainOutputChannelSet();
    return in == out && (out == juce::AudioChannelSet::mono() || out == juce::AudioChannelSet::stereo());
}

void DonswayCleanTakeAudioProcessor::prepareToPlay (double sampleRate, int)
{
    currentSampleRate = sampleRate;
    const int channels = juce::jmax (1, getTotalNumInputChannels());
    noiseEnvelope.assign ((size_t) channels, 0.0f);
    gateEnvelope.assign ((size_t) channels, 0.0f);
    harshEnvelope.assign ((size_t) channels, 0.0f);
    previousInput.assign ((size_t) channels, 0.0f);
    previousHighpassInput.assign ((size_t) channels, 0.0f);
    previousHighpassOutput.assign ((size_t) channels, 0.0f);
    previousLowpassInput.assign ((size_t) channels, 0.0f);
    previousLowpassOutput.assign ((size_t) channels, 0.0f);
    previousHarshInput.assign ((size_t) channels, 0.0f);
    previousHarshOutput.assign ((size_t) channels, 0.0f);
    previousPresenceInput.assign ((size_t) channels, 0.0f);
    previousPresenceOutput.assign ((size_t) channels, 0.0f);
    updateFilters();
}

void DonswayCleanTakeAudioProcessor::releaseResources() {}

void DonswayCleanTakeAudioProcessor::updateFilters()
{
    const float sr = (float) juce::jmax (currentSampleRate, 8000.0);
    const float hp = parameters.getRawParameterValue("rumble")->load();
    const float hpRC = 1.0f / (juce::MathConstants<float>::twoPi * hp);
    highpassA = hpRC / (hpRC + 1.0f / sr);

    // Gentle fixed high-frequency shelf-like low-pass is intentionally omitted;
    // harshness is handled with a dynamic band-limited detector below.
    lowpassA = 1.0f - std::exp (-juce::MathConstants<float>::twoPi * 10500.0f / sr);
    harshA = 1.0f - std::exp (-juce::MathConstants<float>::twoPi * 4200.0f / sr);
    presenceA = 1.0f - std::exp (-juce::MathConstants<float>::twoPi * 8500.0f / sr);
}

float DonswayCleanTakeAudioProcessor::processOneSample (float x, int ch)
{
    updateFilters();
    const float reduction = parameters.getRawParameterValue("clean")->load() * 0.01f;
    const float harshAmount = parameters.getRawParameterValue("harsh")->load() * 0.01f;
    const float gateAmount = parameters.getRawParameterValue("gate")->load() * 0.01f;
    const bool denoiseEnabled = parameters.getRawParameterValue("denoiseOn")->load() > 0.5f;
    const bool harshEnabled = parameters.getRawParameterValue("harshOn")->load() > 0.5f;
    const bool gateEnabled = parameters.getRawParameterValue("gateOn")->load() > 0.5f;

    // First-order high-pass: attenuates sub-bass rumble and handling noise.
    float hp = highpassA * (previousHighpassOutput[(size_t)ch] + x - previousHighpassInput[(size_t)ch]);
    previousHighpassInput[(size_t)ch] = x;
    previousHighpassOutput[(size_t)ch] = hp;

    // Track broad signal level and high-frequency energy with smoothed envelopes.
    const float absX = std::abs (hp);
    const float envCoeff = absX > noiseEnvelope[(size_t)ch] ? 0.012f : 0.00025f;
    noiseEnvelope[(size_t)ch] += envCoeff * (absX - noiseEnvelope[(size_t)ch]);

    const float high = hp - previousLowpassOutput[(size_t)ch];
    previousLowpassOutput[(size_t)ch] += lowpassA * (hp - previousLowpassOutput[(size_t)ch]);
    const float upper = hp - previousPresenceOutput[(size_t)ch];
    previousPresenceOutput[(size_t)ch] += presenceA * (hp - previousPresenceOutput[(size_t)ch]);

    // Dynamic harshness control: estimate high-band energy and attenuate only when
    // upper-mid/high energy is excessive relative to the broadband envelope.
    const float harshBand = hp - previousHarshOutput[(size_t)ch];
    previousHarshOutput[(size_t)ch] += harshA * (hp - previousHarshOutput[(size_t)ch]);
    const float highEnergy = std::abs (harshBand) + 0.35f * std::abs (upper);
    const float speechEnergy = std::abs (previousLowpassOutput[(size_t)ch]) + 0.00003f;
    const float harshTarget = juce::jlimit (0.0f, 1.0f, (highEnergy / speechEnergy - 0.45f) * 0.8f);
    const float hCoeff = harshTarget > harshEnvelope[(size_t)ch] ? 0.02f : 0.001f;
    harshEnvelope[(size_t)ch] += hCoeff * (harshTarget - harshEnvelope[(size_t)ch]);
    if (harshEnabled)
        hp -= harshBand * (harshAmount * 0.55f * harshEnvelope[(size_t)ch]);

    // Lightweight adaptive broadband denoiser: slowly learns the low-level noise
    // floor, then applies a smooth downward expander. This is not spectral AI denoising.
    const float floorCoeff = absX < noiseEnvelope[(size_t)ch] * 1.35f ? 0.0012f : 0.000015f;
    noiseEnvelope[(size_t)ch] += floorCoeff * (absX - noiseEnvelope[(size_t)ch]);
    if (denoiseEnabled)
    {
        const float floor = noiseEnvelope[(size_t)ch] * (0.75f + 1.8f * reduction);
        const float ratio = absX / (floor + 0.00002f);
        const float gain = juce::jlimit (0.22f, 1.0f, (ratio - 0.15f) / (0.9f + reduction));
        hp *= 1.0f - reduction * (1.0f - gain) * 0.85f;
    }

    // Gentle gate/expander with attack and release smoothing to avoid hard chopping.
    if (gateEnabled)
    {
        const float threshold = 0.0007f + gateAmount * 0.006f;
        const float target = absX < threshold ? juce::jlimit (0.0f, 1.0f, absX / threshold) : 1.0f;
        const float coeff = target < gateEnvelope[(size_t)ch] ? 0.006f : 0.00035f;
        gateEnvelope[(size_t)ch] += coeff * (target - gateEnvelope[(size_t)ch]);
        hp *= (1.0f - gateAmount * 0.82f) + gateAmount * 0.82f * gateEnvelope[(size_t)ch];
    }

    previousInput[(size_t)ch] = x;
    return std::isfinite (hp) ? hp : 0.0f;
}

void DonswayCleanTakeAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;
    const int inputs = getTotalNumInputChannels();
    const int outputs = getTotalNumOutputChannels();
    for (int ch = inputs; ch < outputs; ++ch)
        buffer.clear (ch, 0, buffer.getNumSamples());

    for (int ch = 0; ch < inputs; ++ch)
    {
        auto* data = buffer.getWritePointer (ch);
        for (int i = 0; i < buffer.getNumSamples(); ++i)
            data[i] = processOneSample (data[i], ch);
    }
}

juce::AudioProcessorEditor* DonswayCleanTakeAudioProcessor::createEditor()
{
    return new DonswayCleanTakeAudioProcessorEditor (*this);
}

void DonswayCleanTakeAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void DonswayCleanTakeAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xml (getXmlFromBinary (data, sizeInBytes));
    if (xml != nullptr && xml->hasTagName (parameters.state.getType()))
        parameters.replaceState (juce::ValueTree::fromXml (*xml));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new DonswayCleanTakeAudioProcessor();
}
