#include "PluginEditor.h"

PercussionSynthAudioProcessorEditor::PercussionSynthAudioProcessorEditor (PercussionSynthAudioProcessor& p)
    : AudioProcessorEditor (&p), processor (p), gainAttachment (nullptr)
{
    titleLabel.setText ("Percussion Synth", juce::dontSendNotification);
    titleLabel.setFont (juce::Font (18.0f, juce::Font::bold));
    titleLabel.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (titleLabel);

    gainLabel.setText ("Master", juce::dontSendNotification);
    gainLabel.setJustificationType (juce::Justification::centred);
    gainLabel.attachToComponent (&gainSlider, false);
    addAndMakeVisible (gainLabel);

    gainSlider.setSliderStyle (juce::Slider::LinearVertical);
    gainSlider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 60, 20);
    addAndMakeVisible (gainSlider);

    gainAttachment = new juce::SliderParameterAttachment (
        *processor.apvts.getParameter ("masterGain"), gainSlider, nullptr);

    setSize (250, 300);
}

PercussionSynthAudioProcessorEditor::~PercussionSynthAudioProcessorEditor()
{
    delete gainAttachment;
}

void PercussionSynthAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xFF2D2D2D));
    g.setColour (juce::Colours::white);
}

void PercussionSynthAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced (10);
    titleLabel.setBounds (area.removeFromTop (30));
    auto sliderArea = area.reduced (60, 10);
    gainSlider.setBounds (sliderArea.removeFromTop (200));
}
