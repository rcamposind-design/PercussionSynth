#include "PercussionVoice.h"

juce::Array<InstrumentDef> PercussionVoice::instruments;

void PercussionVoice::initInstruments()
{
    if (instruments.size() > 0)
        return;

    instruments.add ({ "Kick",         60.0,  0.9f, 0.3f,  7.0f, 0.12f, 0.002f, 0.35f, 0.0f, 0.05f, 100.0f,  0.0f,   0.0f });
    instruments.add ({ "Snare",       180.0,  0.5f, 0.8f,  0.0f, 0.01f, 0.002f, 0.12f, 0.0f, 0.04f, 2000.0f, 0.0f,   0.0f });
    instruments.add ({ "HiHatClosed", 200.0,  0.1f, 1.0f,  0.0f, 0.01f, 0.001f, 0.04f, 0.0f, 0.01f, 7000.0f, 0.0f,   0.0f });
    instruments.add ({ "HiHatOpen",   200.0,  0.1f, 1.0f,  0.0f, 0.01f, 0.001f, 0.30f, 0.0f, 0.08f, 7000.0f, 0.0f,   0.0f });
    instruments.add ({ "BongoHigh",   360.0,  0.9f, 0.2f,  3.0f, 0.03f, 0.001f, 0.08f, 0.0f, 0.02f, 4000.0f, 0.0f,   0.0f });
    instruments.add ({ "BongoLow",    240.0,  0.9f, 0.2f,  3.0f, 0.04f, 0.001f, 0.10f, 0.0f, 0.03f, 3000.0f, 0.0f,   0.0f });
    instruments.add ({ "CongaHigh",   220.0,  0.8f, 0.3f,  2.0f, 0.06f, 0.004f, 0.18f, 0.0f, 0.08f, 3000.0f, 0.0f,   0.0f });
    instruments.add ({ "CongaLow",    160.0,  0.8f, 0.3f,  2.0f, 0.08f, 0.004f, 0.22f, 0.0f, 0.10f, 2000.0f, 0.0f,   0.0f });
    instruments.add ({ "TimbalesHi",  300.0,  0.8f, 0.3f,  2.0f, 0.05f, 0.003f, 0.15f, 0.0f, 0.06f, 4000.0f, 0.0f,   0.0f });
    instruments.add ({ "TimbalesLo",  200.0,  0.8f, 0.3f,  2.0f, 0.07f, 0.003f, 0.18f, 0.0f, 0.08f, 3000.0f, 0.0f,   0.0f });
    instruments.add ({ "Cabasa",      800.0,  0.2f, 1.0f,  0.0f, 0.01f, 0.010f, 0.12f, 0.0f, 0.04f,  500.0f, 4000.0f, 3000.0f });
    instruments.add ({ "Maracas",     900.0,  0.2f, 1.0f,  0.0f, 0.01f, 0.002f, 0.04f, 0.0f, 0.02f,  800.0f, 5000.0f, 2500.0f });
    instruments.add ({ "Claves",      800.0,  1.0f, 0.0f,  0.0f, 0.01f, 0.001f, 0.02f, 0.0f, 0.01f,  100.0f, 0.0f,   0.0f });
    instruments.add ({ "Tambourine", 2500.0,  0.4f, 0.8f,  0.0f, 0.01f, 0.002f, 0.08f, 0.0f, 0.03f, 4000.0f, 0.0f,   0.0f });
    instruments.add ({ "Clap",        250.0,  0.3f, 1.0f,  0.0f, 0.01f, 0.002f, 0.10f, 0.0f, 0.05f, 1000.0f, 0.0f,   0.0f });
    instruments.add ({ "Shaker",     1500.0,  0.2f, 1.0f,  0.0f, 0.01f, 0.010f, 0.25f, 0.1f, 0.10f,  600.0f, 6000.0f, 2000.0f });
}

const InstrumentDef* PercussionVoice::getInstrumentForNote (int midiNote)
{
    static const int noteMap[] =
    {
        35, 38, 42, 46, 60, 61, 63, 64,
        65, 66, 68, 69, 74, 54, 39, 80
    };
    for (int i = 0; i < instruments.size(); ++i)
        if (noteMap[i] == midiNote)
            return &instruments.getReference (i);
    return nullptr;
}

bool PercussionVoice::canPlaySound (juce::SynthesiserSound* sound)
{
    return dynamic_cast<PercussionSound*> (sound) != nullptr;
}

void PercussionVoice::startNote (int midiNoteNumber, float velocity, juce::SynthesiserSound*, int)
{
    currentDef = getInstrumentForNote (midiNoteNumber);
    if (currentDef == nullptr)
    {
        clearCurrentNote();
        return;
    }

    noteVel = velocity;
    frequency = currentDef->baseFreq;
    phase = 0.0;
    pitchEnv = (double) currentDef->pitchEnvSemitones;
    envAmp = 0.0f;
    noiseHPFPrev = 0.0f;
    noiseBPFPrev1 = 0.0f;
    noiseBPFPrev2 = 0.0f;
    state = StateAttack;
}

void PercussionVoice::stopNote (float /*velocity*/, bool /*allowTailOff*/)
{
    if (state != StateOff)
        state = StateRelease;
}

void PercussionVoice::renderNextBlock (juce::AudioBuffer<float>& buffer, int startSample, int numSamples)
{
    if (state == StateOff || currentDef == nullptr)
        return;

    for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
    {
        float* channelData = buffer.getWritePointer (channel, startSample);
        double sr = getSampleRate();
        if (sr <= 0.0) sr = 44100.0;

        for (int s = 0; s < numSamples; ++s)
        {
            if (state == StateOff)
            {
                channelData[s] = 0.0f;
                continue;
            }

            if (state == StateAttack)
            {
                envAmp += (float) (1.0 / (currentDef->ampAttack * sr));
                if (envAmp >= 1.0f)
                {
                    envAmp = 1.0f;
                    state = StateDecay;
                }
            }
            else if (state == StateDecay)
            {
                envAmp -= (float) (1.0 / (currentDef->ampDecay * sr));
                if (envAmp <= currentDef->ampSustain)
                {
                    envAmp = currentDef->ampSustain;
                    if (currentDef->ampSustain <= 0.0f)
                    {
                        envAmp = 0.0f;
                        state = StateOff;
                        clearCurrentNote();
                        continue;
                    }
                }
            }
            else if (state == StateRelease)
            {
                envAmp -= (float) (1.0 / (currentDef->ampRelease * sr));
                if (envAmp <= 0.0f)
                {
                    envAmp = 0.0f;
                    state = StateOff;
                    clearCurrentNote();
                    continue;
                }
            }

            if (pitchEnv > 0.0)
            {
                pitchEnv -= (double) currentDef->pitchEnvSemitones / (currentDef->pitchEnvDecay * sr);
                if (pitchEnv < 0.0) pitchEnv = 0.0;
            }

            double freq = frequency * std::pow (2.0, pitchEnv / 12.0);

            float osc = generateOscillator (freq, sr);
            float noise = generateNoise (sr);
            float sample = (osc * currentDef->oscLevel + noise * currentDef->noiseLevel) * envAmp * noteVel;

            channelData[s] += sample;
        }
    }
}

float PercussionVoice::generateOscillator (double freq, double sampleRate)
{
    phase += freq / sampleRate;
    if (phase >= 1.0) phase -= 1.0;
    return (float) std::sin (juce::MathConstants<double>::twoPi * phase);
}

float PercussionVoice::generateNoise (double sampleRate)
{
    float n = juce::Random::getSystemRandom().nextFloat() * 2.0f - 1.0f;

    if (currentDef->noiseHPF > 20.0f)
    {
        float rc = 1.0f / (juce::MathConstants<float>::twoPi * currentDef->noiseHPF);
        float dt = 1.0f / (float) sampleRate;
        float alpha = rc / (rc + dt);
        n = alpha * noiseHPFPrev + alpha * (n - noiseHPFPrev);
        noiseHPFPrev = n;
    }

    if (currentDef->noiseBPFreq > 20.0f && currentDef->noiseBW > 0.0f)
    {
        float w0 = juce::MathConstants<float>::twoPi * currentDef->noiseBPFreq / (float) sampleRate;
        float q = currentDef->noiseBPFreq / currentDef->noiseBW;
        float alpha2 = std::sin (w0) / (2.0f * q);
        float cosW0 = std::cos (w0);
        float a0 = 1.0f + alpha2;
        float b0 = alpha2;
        float b2 = -alpha2;
        float a1 = -2.0f * cosW0;
        float a2 = 1.0f - alpha2;

        float out = (b0 / a0) * n + (b2 / a0) * noiseBPFPrev2
                    - (a1 / a0) * noiseBPFPrev1 - (a2 / a0) * noiseBPFPrev2;
        noiseBPFPrev2 = noiseBPFPrev1;
        noiseBPFPrev1 = out;
        n = out;
    }

    return n;
}
