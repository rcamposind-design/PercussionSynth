#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

class PercussionSynthAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    PercussionSynthAudioProcessorEditor (PercussionSynthAudioProcessor&);
    ~PercussionSynthAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    PercussionSynthAudioProcessor& processor;

    juce::Slider gainSlider;
    juce::SliderParameterAttachment* gainAttachment;
    juce::Label gainLabel;
    juce::Label titleLabel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PercussionSynthAudioProcessorEditor)
};
