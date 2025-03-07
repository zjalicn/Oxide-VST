# 🩻 Oxide-VST

![Readme Img](./readme.jpg)

## Advanced Multi-Algorithm Distortion VST

Oxide is a versatile distortion plugin that combines classic analog-style saturation with modern sound design features. Built with surgical precision using JUCE's DSP modules, it offers five distinct distortion algorithms and comprehensive sound shaping capabilities.

### Core Features

- Five distortion algorithms: Soft Clip, Hard Clip, Foldback, Waveshaper, and Bitcrusher
- Real-time waveform and spectrum visualization
- Advanced oversampling (2x/4x/8x) with high-quality IIR filtering
- Pre and post filters with variable resonance
- Modulation system with LFO and envelope follower
- Mid/Side processing with stereo width control
- Input/Output gain staging
- 64-bit processing
- Built in C++ using JUCE framework

## Installation

1. Clone the repository
2. Run

   ```
   git submodule add https://github.com/juce-framework/JUCE.git
   git submodule update --init --recursive
   ```

3. Build the project

   ```
   cmake .
   cmake --build .
   ```

4. Copy the plugin to your VST folder

5. Afterwards, Clean and Rebuild with:

   ```
    rm -rf build
    mkdir build
    cd build
    cmake ..
    cmake --build .
   ```
