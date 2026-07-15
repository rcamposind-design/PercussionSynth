#pragma once
#include <JuceHeader.h>

struct InstrumentDef
{
    juce::String name;
    double baseFreq;
    float oscLevel;
    float noiseLevel;
    float pitchEnvSemitones;
    float pitchEnvDecay;
    float ampAttack;
    float ampDecay;
    float ampSustain;
    float ampRelease;
    float noiseHPF;
    float noiseBPFreq;
    float noiseBW;
};

class PercussionSound : public juce::SynthesiserSound
{
public:
    bool appliesToNote (int) override { return true; }
    bool appliesToChannel (int) override { return true; }
};

class PercussionVoice : public juce::SynthesiserVoice
{
public:
    bool canPlaySound (juce::SynthesiserSound* sound) override;
    void startNote (int midiNoteNumber, float velocity, juce::SynthesiserSound* sound, int currentPitchWheelPosition) override;
    void stopNote (float velocity, bool allowTailOff) override;
    void renderNextBlock (juce::AudioBuffer<float>& outputBuffer, int startSample, int numSamples) override;
    void pitchWheelMoved (int /*newPitchWheelValue*/) override {}
    void controllerMoved (int /*controllerNumber*/, int /*newValue*/) override {}

    static void initInstruments();
    static const InstrumentDef* getInstrumentForNote (int midiNote);

private:
    static juce::Array<InstrumentDef> instruments;

    double frequency = 0.0;
    float noteVel = 0.0;
    double phase = 0.0;
    double pitchEnv = 0.0;
    float envAmp = 0.0;

    float noiseHPFPrev = 0.0f;
    float noiseBPFPrev1 = 0.0f;
    float noiseBPFPrev2 = 0.0f;

    enum { StateOff, StateAttack, StateDecay, StateRelease };
    int state = StateOff;

    const InstrumentDef* currentDef = nullptr;

    float generateOscillator (double freq, double sampleRate);
    float generateNoise (double sampleRate);
};
