#include "MeterView.h"
#include "BinaryData.h"

class MeterViewBrowserComponent : public juce::WebBrowserComponent
{
public:
    MeterViewBrowserComponent(MeterView &owner)
        : ownerView(owner) {}

    bool pageAboutToLoad(const juce::String &url) override
    {
        // Handle oxide: protocol for control messages
        if (url.startsWith("oxide:"))
        {
            juce::String params = url.fromFirstOccurrenceOf("oxide:", false, true);

            if (params.startsWith("inputGain="))
            {
                float value = params.fromFirstOccurrenceOf("inputGain=", false, true).getFloatValue();
                ownerView.notifyInputGainChanged(value);
                return false;
            }
            else if (params.startsWith("outputGain="))
            {
                float value = params.fromFirstOccurrenceOf("outputGain=", false, true).getFloatValue();
                ownerView.notifyOutputGainChanged(value);
                return false;
            }

            return false; // We handled this URL
        }

        return true; // We didn't handle this URL
    }

private:
    MeterView &ownerView;
};

MeterView::MeterView()
    : inputGain(0.0f), outputGain(0.0f),
      lastLeftLevel(0.0f), lastRightLevel(0.0f),
      outLeftLevel(0.0f), outRightLevel(0.0f),
      pageLoaded(false)
{
    // Create the browser with the resource handler
    auto browser = new MeterViewBrowserComponent(*this);
    webView.reset(browser);
    webView->setFocusContainer(false);
    addAndMakeVisible(webView.get());

    // Get the HTML content
    juce::String htmlContent = juce::String(BinaryData::meters_html, BinaryData::meters_htmlSize);

    // Inject CSS directly into HTML head
    juce::String cssContent = juce::String(BinaryData::meters_css, BinaryData::meters_cssSize);
    htmlContent = htmlContent.replace(
        "<link rel=\"stylesheet\" href=\"./global.css\" />",
        "<style>" + cssContent + "</style>");

    // Load the combined HTML content
    webView->goToURL("data:text/html;charset=utf-8," + htmlContent);

    // Start a timer to periodically check the page status
    startTimer(50);
}

MeterView::~MeterView()
{
    stopTimer();
    webView = nullptr;
}

void MeterView::timerCallback()
{
    static int loadingCounter = 0;
    loadingCounter++;

    // After a short delay, assume page is loaded and set flag
    if (loadingCounter >= 5)
    { // About 250ms
        stopTimer();
        pageLoaded = true;

        // Force an initial update with explicit zero values to ensure meters are empty
        juce::String script = "window.setAudioLevels(0, 0, 0, 0)";
        webView->evaluateJavascript(script);

        // Also explicitly update our internal state
        lastLeftLevel = 0.0f;
        lastRightLevel = 0.0f;
        outLeftLevel = 0.0f;
        outRightLevel = 0.0f;
    }
}

void MeterView::paint(juce::Graphics &g)
{
    // Nothing to paint here - WebBrowserComponent handles rendering
}

void MeterView::resized()
{
    webView->setBounds(getLocalBounds());
}

void MeterView::updateLevels(float leftLevel, float rightLevel)
{
    if (!pageLoaded)
        return;

    // Store the last known levels
    lastLeftLevel = leftLevel;
    lastRightLevel = rightLevel;

    // If signal is very close to zero, explicitly set it to zero to ensure bars disappear
    if (leftLevel < 0.01f)
        leftLevel = 0.0f;
    if (rightLevel < 0.01f)
        rightLevel = 0.0f;

    // Calculate output levels based on input levels and output gain
    float gainMultiplier = std::pow(10.0f, outputGain / 20.0f);
    outLeftLevel = leftLevel * gainMultiplier;
    outRightLevel = rightLevel * gainMultiplier;

    // Also ensure output can be completely empty
    if (outLeftLevel < 0.01f)
        outLeftLevel = 0.0f;
    if (outRightLevel < 0.01f)
        outRightLevel = 0.0f;

    // Clamp output levels to 0-100%
    outLeftLevel = juce::jlimit(0.0f, 100.0f, outLeftLevel);
    outRightLevel = juce::jlimit(0.0f, 100.0f, outRightLevel);

    // Use try-catch to handle potential JavaScript errors
    try
    {
        // Ensure values are valid by using String conversion with proper decimal formatting
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

        // Fall back to the older, simpler method if available
        juce::String fallbackScript = "window.setAudioLevels(" +
                                      juce::String(leftLevel, 1) + ", " +
                                      juce::String(rightLevel, 1) + ")";
        webView->evaluateJavascript(fallbackScript);
    }
}

void MeterView::setInputGain(float newGain)
{
    inputGain = newGain;
    // Force an update of the UI with current levels but new gain settings
    updateLevels(lastLeftLevel, lastRightLevel);
}

void MeterView::setOutputGain(float newGain)
{
    outputGain = newGain;
    // Force an update of the UI with current levels but new gain settings
    updateLevels(lastLeftLevel, lastRightLevel);
}

void MeterView::notifyInputGainChanged(float newGain)
{
    inputGain = newGain;
    if (onInputGainChanged)
        onInputGainChanged(newGain);

    // Update display with new gain value
    updateLevels(lastLeftLevel, lastRightLevel);
}

void MeterView::notifyOutputGainChanged(float newGain)
{
    outputGain = newGain;
    if (onOutputGainChanged)
        onOutputGainChanged(newGain);

    // Update display with new gain value
    updateLevels(lastLeftLevel, lastRightLevel);
}