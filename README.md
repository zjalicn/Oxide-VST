# 🩻 Oxide-VST

![Readme Img](./readme.jpg)

## Oxide

Oxide is a distortion + delay plugin that combines classic analog-style saturation with repeating echoes. Built with surgical precision using JUCE's DSP modules, it offers five distinct distortion algorithms and comprehensive sound shaping capabilities.

- Built in C++ using JUCE framework
- Compiled with CMake
- WebView Component Integration for UI
- SASS for better organization and a templated theming system

### Features

- Five distortion algorithms: Soft Clip, Hard Clip, Foldback, Waveshaper, and Bitcrusher
- Real-time waveform and spectrum visualization
- Input/Output gain staging

### Future Features

##### Tech

- templated html
  - knockout? or some caveman simple option
  - react-juce?
  - transparent webview components?

##### Features

- Pre and post filters with variable resonance
- Modulation system with LFO and envelope follower
- Mid/Side processing with stereo width control

## Installation

1. Clone the repository
2. Run

   ```
   git submodule add https://github.com/juce-framework/JUCE.git
   git submodule update --init --recursive
   ```

3. Build the project

   ```
   cmake -B build (first time only i think?)
   cmake --build build
   ```

4. Copy the plugin to your VST folder

5. Afterwards, Clean and Rebuild with:

   ```
   rm -rf build
   cmake -B build
   cmake --build build
   ```
