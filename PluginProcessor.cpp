#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "PercussionVoice.h"

PercussionSynthAudioProcessor::PercussionSynthAudioProcessor()
    : AudioProcessor (BusesProperties().withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "Parameters", createParameterLayout())
{
    PercussionVoice::initInstruments();

    for (int i = 0; i < 16; ++i)
        synth.addVoice (new PercussionVoice());

    synth.addSound (new PercussionSound());
}

PercussionSynthAudioProcessor::~PercussionSynthAudioProcessor() {}

juce::AudioProcessorValueTreeState::ParameterLayout PercussionSynthAudioProcessor::createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;
    layout.add (std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { "masterGain", 1 }, "Master Gain",
        juce::NormalisableRange<float> (0.0f, 1.0f, 0.01f), 0.75f));
    return layout;
}

void PercussionSynthAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    synth.setCurrentPlaybackSampleRate (sampleRate);
}

void PercussionSynthAudioProcessor::releaseResources() {}

void PercussionSynthAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumOutputChannels; i < buffer.getNumChannels(); ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    synth.renderNextBlock (buffer, midiMessages, 0, buffer.getNumSamples());

    float gain = apvts.getRawParameterValue ("masterGain")->load();
    for (int channel = 0; channel < totalNumOutputChannels; ++channel)
        buffer.applyGain (channel, 0, buffer.getNumSamples(), gain);
}

juce::AudioProcessorEditor* PercussionSynthAudioProcessor::createEditor()
{
    return new PercussionSynthAudioProcessorEditor (*this);
}

void PercussionSynthAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void PercussionSynthAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xml (getXmlFromBinary (data, sizeInBytes));
    if (xml != nullptr && xml->hasTagName (apvts.state.getType()))
        apvts.replaceState (juce::ValueTree::fromXml (*xml));
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new PercussionSynthAudioProcessor();
}
