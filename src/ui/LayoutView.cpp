#include "LayoutView.h"
#include "BinaryData.h"

LayoutView::LayoutMessageHandler::LayoutMessageHandler(LayoutView &owner)
    : ownerView(owner)
{
}

bool LayoutView::LayoutMessageHandler::pageAboutToLoad(const juce::String &url)
{
    // Handle oxide: protocol for control messages
    if (url.startsWith("oxide:"))
    {
        juce::String params = url.fromFirstOccurrenceOf("oxide:", false, true);

        // Handle preset selection
        if (params.startsWith("preset="))
        {
            juce::String presetName = params.fromFirstOccurrenceOf("preset=", false, true);
            if (ownerView.onPresetSelected)
                ownerView.onPresetSelected(presetName);
            return false;
        }
        // Handle save action
        else if (params.startsWith("action=save"))
        {
            if (ownerView.onSaveClicked)
                ownerView.onSaveClicked();
            return false;
        }
        // Handle input gain change
        else if (params.startsWith("inputGain="))
        {
            float value = params.fromFirstOccurrenceOf("inputGain=", false, true).getFloatValue();
            ownerView.setInputGain(value);
            if (ownerView.onInputGainChanged)
                ownerView.onInputGainChanged(value);
            return false;
        }
        // Handle output gain change
        else if (params.startsWith("outputGain="))
        {
            float value = params.fromFirstOccurrenceOf("outputGain=", false, true).getFloatValue();
            ownerView.setOutputGain(value);
            if (ownerView.onOutputGainChanged)
                ownerView.onOutputGainChanged(value);
            return false;
        }
        // Handle distortion parameters
        else if (params.startsWith("drive="))
        {
            float value = params.fromFirstOccurrenceOf("drive=", false, true).getFloatValue();
            ownerView.distortionProcessor.setDrive(value);
            return false;
        }
        else if (params.startsWith("mix="))
        {
            float value = params.fromFirstOccurrenceOf("mix=", false, true).getFloatValue();
            ownerView.distortionProcessor.setMix(value);
            return false;
        }
        else if (params.startsWith("algorithm="))
        {
            juce::String value = params.fromFirstOccurrenceOf("algorithm=", false, true);
            ownerView.distortionProcessor.setAlgorithm(value);
            return false;
        }
        // Handle delay parameters
        else if (params.startsWith("delay:"))
        {
            params = params.fromFirstOccurrenceOf("delay:", false, true);

            if (params.startsWith("time="))
            {
                float value = params.fromFirstOccurrenceOf("time=", false, true).getFloatValue();
                ownerView.delayProcessor.setDelayTime(value);
                return false;
            }
            else if (params.startsWith("feedback="))
            {
                float value = params.fromFirstOccurrenceOf("feedback=", false, true).getFloatValue();
                ownerView.delayProcessor.setFeedback(value);
                return false;
            }
            else if (params.startsWith("mix="))
            {
                float value = params.fromFirstOccurrenceOf("mix=", false, true).getFloatValue();
                ownerView.delayProcessor.setMix(value);
                return false;
            }
            else if (params.startsWith("pingpong="))
            {
                int value = params.fromFirstOccurrenceOf("pingpong=", false, true).getIntValue();
                ownerView.delayProcessor.setPingPong(value > 0);
                return false;
            }
        }
        // Handle filter parameters
        else if (params.startsWith("filter:"))
        {
            params = params.fromFirstOccurrenceOf("filter:", false, true);

            if (params.startsWith("type="))
            {
                juce::String value = params.fromFirstOccurrenceOf("type=", false, true);
                ownerView.filterProcessor.setFilterType(value);
                return false;
            }
            else if (params.startsWith("frequency="))
            {
                float value = params.fromFirstOccurrenceOf("frequency=", false, true).getFloatValue();
                ownerView.filterProcessor.setFrequency(value);
                return false;
            }
            else if (params.startsWith("resonance="))
            {
                float value = params.fromFirstOccurrenceOf("resonance=", false, true).getFloatValue();
                ownerView.filterProcessor.setResonance(value);
                return false;
            }
        }
        // Handle pulse parameters
        else if (params.startsWith("pulse:"))
        {
            params = params.fromFirstOccurrenceOf("pulse:", false, true);

            if (params.startsWith("mix="))
            {
                float value = params.fromFirstOccurrenceOf("mix=", false, true).getFloatValue();
                ownerView.pulseProcessor.setMix(value);
                return false;
            }
            else if (params.startsWith("rate="))
            {
                juce::String value = params.fromFirstOccurrenceOf("rate=", false, true);
                ownerView.pulseProcessor.setRate(value);
                return false;
            }
        }

        return false; // We handled this URL
    }

    return true; // We didn't handle this URL
}

// Main LayoutView implementation
LayoutView::LayoutView(DistortionProcessor &distProc, DelayProcessor &delayProc, FilterProcessor &filterProc, PulseProcessor &pulseProc)
    : distortionProcessor(distProc),
      delayProcessor(delayProc),
      filterProcessor(filterProc),
      pulseProcessor(pulseProc),
      pageLoaded(false),
      inputGain(0.0f),
      outputGain(0.0f),
      lastLeftLevel(0.0f),
      lastRightLevel(0.0f),
      lastDrive(distProc.getDrive()),
      lastMix(distProc.getMix()),
      lastAlgorithm(distProc.getAlgorithmName()),
      lastDelayTime(delayProc.getDelayTime()),
      lastFeedback(delayProc.getFeedback()),
      lastDelayMix(delayProc.getMix()),
      lastPingPong(delayProc.getPingPong()),
      lastFilterType(filterProc.getFilterTypeName()),
      lastFilterFreq(filterProc.getFrequency()),
      lastResonance(filterProc.getResonance()),
      lastPulseMix(pulseProc.getMix()),
      lastPulseRate(pulseProc.getRateString())
{
    auto browser = new LayoutMessageHandler(*this);
    webView.reset(browser);
    webView->setFocusContainer(false);
    addAndMakeVisible(webView.get());

    juce::String htmlContent = juce::String(BinaryData::layout_html, BinaryData::layout_htmlSize);
    juce::String cssContent = juce::String(BinaryData::layout_css, BinaryData::layout_cssSize);

    htmlContent = htmlContent.replace(
        "<link rel=\"stylesheet\" href=\"./layout.css\" />",
        "<style>\n" + cssContent + "\n    </style>");

    // Load the combined HTML content
    webView->goToURL("data:text/html;charset=utf-8," + htmlContent);

    // Start timer for updates
    startTimerHz(30);
}

LayoutView::~LayoutView()
{
    stopTimer();
    webView = nullptr;
}

void LayoutView::paint(juce::Graphics &g)
{
    // Nothing to paint - WebView handles rendering
}

void LayoutView::resized()
{
    // Make the webView take up all available space
    webView->setBounds(getLocalBounds());
}

void LayoutView::timerCallback()
{
    // First few cycles, just wait for page to load
    static int pageLoadCounter = 0;
    if (!pageLoaded)
    {
        pageLoadCounter++;
        if (pageLoadCounter >= 10) // About 333ms with 30Hz timer
        {
            pageLoaded = true;

            // Initialize with default values
            juce::String script = "window.setAudioState(0, 0, 0, 0, " +
                                  juce::String(inputGain) + ", " +
                                  juce::String(outputGain) + ")";
            webView->evaluateJavascript(script);
        }
        return;
    }

    // Check for parameter changes in distortion processor
    float drive = distortionProcessor.getDrive();
    float mix = distortionProcessor.getMix();
    juce::String algorithm = distortionProcessor.getAlgorithmName();

    bool distortionChanged = std::abs(drive - lastDrive) > 0.001f ||
                             std::abs(mix - lastMix) > 0.001f ||
                             algorithm != lastAlgorithm;

    if (distortionChanged)
    {
        // Update distortion UI with current values
        juce::String script = "window.setDistortionValues(" +
                              juce::String(drive) + ", " +
                              juce::String(mix) + ", '" +
                              algorithm + "')";
        webView->evaluateJavascript(script);

        lastDrive = drive;
        lastMix = mix;
        lastAlgorithm = algorithm;
    }

    // Check for parameter changes in delay processor
    float delayTime = delayProcessor.getDelayTime();
    float feedback = delayProcessor.getFeedback();
    float delayMix = delayProcessor.getMix();
    bool pingPong = delayProcessor.getPingPong();

    bool delayChanged = std::abs(delayTime - lastDelayTime) > 0.001f ||
                        std::abs(feedback - lastFeedback) > 0.001f ||
                        std::abs(delayMix - lastDelayMix) > 0.001f ||
                        pingPong != lastPingPong;

    if (delayChanged)
    {
        // Update delay UI with current values
        juce::String script = "window.setDelayValues(" +
                              juce::String(delayTime) + ", " +
                              juce::String(feedback) + ", " +
                              juce::String(delayMix) + ", " +
                              juce::String(pingPong ? "1" : "0") + ")";
        webView->evaluateJavascript(script);

        lastDelayTime = delayTime;
        lastFeedback = feedback;
        lastDelayMix = delayMix;
        lastPingPong = pingPong;
    }

    // Check for parameter changes in filter processor
    juce::String filterType = filterProcessor.getFilterTypeName();
    float filterFreq = filterProcessor.getFrequency();
    float resonance = filterProcessor.getResonance();

    bool filterChanged = filterType != lastFilterType ||
                         std::abs(filterFreq - lastFilterFreq) > 0.001f ||
                         std::abs(resonance - lastResonance) > 0.001f;

    if (filterChanged)
    {
        // Update filter UI with current values
        juce::String script = "window.setFilterValues('" +
                              filterType + "', " +
                              juce::String(filterFreq) + ", " +
                              juce::String(resonance) + ")";
        webView->evaluateJavascript(script);

        lastFilterType = filterType;
        lastFilterFreq = filterFreq;
        lastResonance = resonance;
    }

    // Check for parameter changes in pulse processor
    float pulseMix = pulseProcessor.getMix();
    juce::String pulseRate = pulseProcessor.getRateString();
    bool pulseChanged = std::abs(pulseMix - lastPulseMix) > 0.001f || pulseRate != lastPulseRate;

    if (pulseChanged)
    {
        // Update pulse UI with current values
        juce::String script = "window.setPulseValues(" +
                              juce::String(pulseMix) + ", '" +
                              pulseRate + "', " +
                              juce::String(pulseProcessor.getBpm()) + ")";
        webView->evaluateJavascript(script);

        lastPulseMix = pulseMix;
        lastPulseRate = pulseRate;
    }

    // Update oscilloscope if there's new audio data
    {
        juce::ScopedLock lock(bufferLock);
        if (latestBuffer.getNumChannels() > 0 && latestBuffer.getNumSamples() > 0)
        {
            juce::String dataJson = prepareWaveformData();
            juce::String script = "window.updateOscilloscopeData(" + dataJson + ")";
            webView->evaluateJavascript(script);
        }
    }
}

void LayoutView::updateBuffer(const juce::AudioBuffer<float> &buffer)
{
    juce::ScopedLock lock(bufferLock);

    // Update our buffer copy with the latest audio data
    if (buffer.getNumChannels() == 0 || buffer.getNumSamples() == 0)
        return;

    latestBuffer.makeCopyOf(buffer);
}

void LayoutView::updateLevels(float leftLevel, float rightLevel, float outLeftLevel, float outRightLevel)
{
    if (!pageLoaded)
        return;

    // Store the last known levels
    lastLeftLevel = leftLevel;
    lastRightLevel = rightLevel;

    // If signal is very close to zero, explicitly set it to zero
    if (leftLevel < 0.01f)
        leftLevel = 0.0f;
    if (rightLevel < 0.01f)
        rightLevel = 0.0f;
    if (outLeftLevel < 0.01f)
        outLeftLevel = 0.0f;
    if (outRightLevel < 0.01f)
        outRightLevel = 0.0f;

    try
    {
        // Ensure values are valid by using String conversion with proper formatting
        juce::String script = "window.setAudioState(" +
                              juce::String(leftLevel, 1) + ", " +
                              juce::String(rightLevel, 1) + ", " +
                              juce::String(outLeftLevel, 1) + ", " +
                              juce::String(outRightLevel, 1) + ", " +
                              juce::String(inputGain, 1) + ", " +
                              juce::String(outputGain, 1) + ")";

        webView->evaluateJavascript(script);
    }
    catch (const std::exception &e)
    {
        // Log any errors for debugging
        juce::Logger::writeToLog("JavaScript error in meters: " + juce::String(e.what()));
    }
}

void LayoutView::setInputGain(float newGain)
{
    inputGain = newGain;
    // Force an update of the UI with current levels but new gain settings
    updateLevels(lastLeftLevel, lastRightLevel, 0.0f, 0.0f);
}

void LayoutView::setOutputGain(float newGain)
{
    outputGain = newGain;
    // Force an update of the UI with current levels but new gain settings
    updateLevels(lastLeftLevel, lastRightLevel, 0.0f, 0.0f);
}

juce::String LayoutView::prepareWaveformData()
{
    juce::ScopedLock lock(bufferLock);

    // Number of data points to display
    const int numPoints = 128;

    if (latestBuffer.getNumChannels() == 0 || latestBuffer.getNumSamples() == 0)
        return "[]";

    // Create a JSON array of waveform data points
    juce::String jsonArray = "[";

    // Combine channels if stereo
    if (latestBuffer.getNumChannels() > 1)
    {
        const float *leftChannel = latestBuffer.getReadPointer(0);
        const float *rightChannel = latestBuffer.getReadPointer(1);

        // Sample the buffer at regular intervals
        const int samplesPerPoint = juce::jmax(1, latestBuffer.getNumSamples() / numPoints);

        for (int i = 0; i < numPoints && i * samplesPerPoint < latestBuffer.getNumSamples(); ++i)
        {
            // Average the samples in this segment
            float sum = 0.0f;
            int count = 0;

            for (int j = 0; j < samplesPerPoint && i * samplesPerPoint + j < latestBuffer.getNumSamples(); ++j)
            {
                int sampleIndex = i * samplesPerPoint + j;
                // Average of left and right channels
                sum += (leftChannel[sampleIndex] + rightChannel[sampleIndex]) * 0.5f;
                count++;
            }

            // Calculate average value for this point
            float value = count > 0 ? sum / count : 0.0f;

            // Add to JSON array
            jsonArray += juce::String(value);

            // Add comma if not the last item
            if (i < numPoints - 1 && (i + 1) * samplesPerPoint < latestBuffer.getNumSamples())
                jsonArray += ",";
        }
    }
    else // Mono
    {
        const float *channel = latestBuffer.getReadPointer(0);

        // Sample the buffer at regular intervals
        const int samplesPerPoint = juce::jmax(1, latestBuffer.getNumSamples() / numPoints);

        for (int i = 0; i < numPoints && i * samplesPerPoint < latestBuffer.getNumSamples(); ++i)
        {
            // Average the samples in this segment
            float sum = 0.0f;
            int count = 0;

            for (int j = 0; j < samplesPerPoint && i * samplesPerPoint + j < latestBuffer.getNumSamples(); ++j)
            {
                sum += channel[i * samplesPerPoint + j];
                count++;
            }

            // Calculate average value for this point
            float value = count > 0 ? sum / count : 0.0f;

            // Add to JSON array
            jsonArray += juce::String(value);

            // Add comma if not the last item
            if (i < numPoints - 1 && (i + 1) * samplesPerPoint < latestBuffer.getNumSamples())
                jsonArray += ",";
        }
    }

    jsonArray += "]";
    return jsonArray;
}

void LayoutView::updatePresetList(const juce::StringArray &presets)
{
    // Store the preset list
    presetList = presets;

    // Only update if page is loaded
    if (!pageLoaded)
        return;

    try
    {
        // Create a JavaScript array string for the presets
        juce::String jsArrayString = "[";

        for (int i = 0; i < presets.size(); ++i)
        {
            juce::String displayName = presets[i];
            juce::String valueName = displayName.replaceCharacters(" ", "_").toLowerCase();

            jsArrayString += "{\"value\":\"" + valueName + "\", \"text\":\"" + displayName + "\"}";
            if (i < presets.size() - 1)
                jsArrayString += ", ";
        }

        jsArrayString += "]";

        // Create JavaScript to update the dropdown
        juce::String script =
            "const presetDropdown = document.getElementById('presetDropdown');\n"
            "// Clear existing options\n"
            "presetDropdown.innerHTML = '';\n"
            "\n"
            "// Add new preset options\n"
            "const presets = " +
            jsArrayString + ";\n"
                            "presets.forEach(preset => {\n"
                            "    const option = document.createElement('option');\n"
                            "    option.value = preset.value;\n" // Use underscore value
                            "    option.text = preset.text;\n"   // Use display text with spaces
                            "    presetDropdown.appendChild(option);\n"
                            "});\n";

        // Evaluate the JavaScript to update the dropdown
        webView->evaluateJavascript(script, nullptr);
    }
    catch (const std::exception &)
    {
        // Silently handle any JavaScript errors
    }
}

void LayoutView::refreshAllParameters()
{
    // Force an immediate refresh of all parameters

    // Distortion parameters
    {
        float drive = distortionProcessor.getDrive();
        float mix = distortionProcessor.getMix();
        juce::String algorithm = distortionProcessor.getAlgorithmName();

        juce::String script = "window.setDistortionValues(" +
                              juce::String(drive) + ", " +
                              juce::String(mix) + ", '" +
                              algorithm + "')";
        webView->evaluateJavascript(script);

        lastDrive = drive;
        lastMix = mix;
        lastAlgorithm = algorithm;
    }

    // Delay parameters
    {
        float delayTime = delayProcessor.getDelayTime();
        float feedback = delayProcessor.getFeedback();
        float delayMix = delayProcessor.getMix();
        bool pingPong = delayProcessor.getPingPong();

        juce::String script = "window.setDelayValues(" +
                              juce::String(delayTime) + ", " +
                              juce::String(feedback) + ", " +
                              juce::String(delayMix) + ", " +
                              juce::String(pingPong ? "1" : "0") + ")";
        webView->evaluateJavascript(script);

        lastDelayTime = delayTime;
        lastFeedback = feedback;
        lastDelayMix = delayMix;
        lastPingPong = pingPong;
    }

    // Filter parameters
    {
        juce::String filterType = filterProcessor.getFilterTypeName();
        float filterFreq = filterProcessor.getFrequency();
        float resonance = filterProcessor.getResonance();

        juce::String script = "window.setFilterValues('" +
                              filterType + "', " +
                              juce::String(filterFreq) + ", " +
                              juce::String(resonance) + ")";
        webView->evaluateJavascript(script);

        lastFilterType = filterType;
        lastFilterFreq = filterFreq;
        lastResonance = resonance;
    }

    // Pulse parameters
    {
        float pulseMix = pulseProcessor.getMix();
        juce::String pulseRate = pulseProcessor.getRateString();

        juce::String script = "window.setPulseValues(" +
                              juce::String(pulseMix) + ", '" +
                              pulseRate + "', " +
                              juce::String(pulseProcessor.getBpm()) + ")";
        webView->evaluateJavascript(script);

        lastPulseMix = pulseMix;
        lastPulseRate = pulseRate;
    }

    // Update levels
    updateLevels(lastLeftLevel, lastRightLevel, 0.0f, 0.0f);
}