#pragma once

namespace fls::synth {

enum class Waveform { Sine, Saw, Square, Noise };

class Oscillator {
public:
    void prepare(double sampleRate);
    void setWaveform(Waveform waveform);
    void setFrequency(float frequencyHz);
    float nextSample();

private:
    double m_sampleRate {48000.0};
    Waveform m_waveform {Waveform::Saw};
    float m_frequency {440.0f};
    float m_phase {0.0f};
    std::uint32_t m_noiseState {0x12345678};
};

} // namespace fls::synth
