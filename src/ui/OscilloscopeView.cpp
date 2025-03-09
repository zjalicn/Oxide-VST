#include "OscilloscopeView.h"
#include "BinaryData.h"

OscilloscopeView::OscilloscopeView(DistortionProcessor &processor)
    : distortionProcessor(processor),
      pageLoaded(false)
{
    webView = std::make_unique<juce::WebBrowserComponent>();
    webView->setFocusContainer(false);
    addAndMakeVisible(webView.get());

    juce::String htmlContent = juce::String(BinaryData::oscilloscope_html, BinaryData::oscilloscope_htmlSize);
    juce::String cssContent = juce::String(BinaryData::oscilloscope_css, BinaryData::oscilloscope_cssSize);
    htmlContent = htmlContent.replace(
        "<link rel=\"stylesheet\" href=\"./global.css\" />",
        "<style>" + cssContent + "</style>");

    // Load the combined HTML content
    webView->goToURL("data:text/html;charset=utf-8," + htmlContent);

    startTimerHz(30);
}

OscilloscopeView::~OscilloscopeView()
{
    stopTimer();
    webView = nullptr;
}

void OscilloscopeView::paint(juce::Graphics &g)
{
}

void OscilloscopeView::resized()
{
    webView->setBounds(getLocalBounds());
}

void OscilloscopeView::timerCallback()
{
    if (!pageLoaded) // Wait for the page to load before sending data
    {
        static int counter = 0;
        counter++;
        if (counter >= 10)
            pageLoaded = true;
        else
            return;
    }

    // Update the waveform data
    juce::String dataJson = prepareWaveformData();
    juce::String script = "window.updateOscilloscopeData(" + dataJson + ")";
    webView->evaluateJavascript(script);
}

void OscilloscopeView::updateBuffer(const juce::AudioBuffer<float> &buffer)
{
    juce::ScopedLock lock(bufferLock);

    // Update our buffer copy with the latest audio data
    if (buffer.getNumChannels() == 0 || buffer.getNumSamples() == 0)
        return;

    latestBuffer.makeCopyOf(buffer);
}

juce::String OscilloscopeView::prepareWaveformData()
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