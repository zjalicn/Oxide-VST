#include "DelayControlPanel.h"
#include "BinaryData.h"

class DelayBrowserComponent : public juce::WebBrowserComponent
{
public:
    DelayBrowserComponent(DelayProcessor &proc)
        : processor(proc) {}

    bool pageAboutToLoad(const juce::String &url) override
    {
        // Handle oxide:delay: protocol for control messages
        if (url.startsWith("oxide:delay:"))
        {
            juce::String params = url.fromFirstOccurrenceOf("oxide:delay:", false, true);

            if (params.startsWith("time="))
            {
                float value = params.fromFirstOccurrenceOf("time=", false, true).getFloatValue();
                processor.setDelayTime(value);
                return false;
            }
            else if (params.startsWith("feedback="))
            {
                float value = params.fromFirstOccurrenceOf("feedback=", false, true).getFloatValue();
                processor.setFeedback(value);
                return false;
            }
            else if (params.startsWith("mix="))
            {
                float value = params.fromFirstOccurrenceOf("mix=", false, true).getFloatValue();
                processor.setMix(value);
                return false;
            }
            else if (params.startsWith("filter="))
            {
                float value = params.fromFirstOccurrenceOf("filter=", false, true).getFloatValue();
                processor.setFilterFreq(value);
                return false;
            }
            else if (params.startsWith("pingpong="))
            {
                int value = params.fromFirstOccurrenceOf("pingpong=", false, true).getIntValue();
                processor.setPingPong(value > 0);
                return false;
            }

            return false; // We handled this URL
        }

        return true; // We didn't handle this URL
    }

private:
    DelayProcessor &processor;
};

DelayControlPanel::DelayControlPanel(DelayProcessor &processor)
    : delayProcessor(processor),
      lastDelayTime(processor.getDelayTime()),
      lastFeedback(processor.getFeedback()),
      lastMix(processor.getMix()),
      lastFilterFreq(processor.getFilterFreq()),
      lastPingPong(processor.getPingPong())
{
    // Create the browser with the resource handler
    auto browser = new DelayBrowserComponent(delayProcessor);
    webView.reset(browser);
    webView->setOpaque(false);
    webView->setWantsKeyboardFocus(false);
    webView->setFocusContainer(false);
    addAndMakeVisible(webView.get());

    // Get the HTML content
    juce::String htmlContent = juce::String(BinaryData::fxdelay_html, BinaryData::fxdelay_htmlSize);

    // Inject CSS directly into HTML head
    juce::String cssContent = juce::String(BinaryData::fxdelay_css, BinaryData::fxdelay_cssSize);

    // Add viewport meta tag for better sizing
    htmlContent = htmlContent.replace(
        "<head>",
        "<head>\n<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no\">");

    htmlContent = htmlContent.replace(
        "<link rel=\"stylesheet\" href=\"./global.css\" />",
        "<style>" + cssContent + "</style>");

    // Load the combined HTML content
    webView->goToURL("data:text/html;charset=utf-8," + htmlContent);

    startTimer(50);
}

DelayControlPanel::~DelayControlPanel()
{
    webView = nullptr;
    stopTimer();
}

void DelayControlPanel::paint(juce::Graphics &g)
{
    // Nothing to paint here - WebBrowserComponent handles rendering
}

void DelayControlPanel::resized()
{
    // Make sure the webView takes up the exact same area as this component
    webView->setBounds(getLocalBounds());
}

void DelayControlPanel::timerCallback()
{
    float delayTime = delayProcessor.getDelayTime();
    float feedback = delayProcessor.getFeedback();
    float mix = delayProcessor.getMix();
    float filterFreq = delayProcessor.getFilterFreq();
    bool pingPong = delayProcessor.getPingPong();

    bool valuesChanged = std::abs(delayTime - lastDelayTime) > 0.001f ||
                         std::abs(feedback - lastFeedback) > 0.001f ||
                         std::abs(mix - lastMix) > 0.001f ||
                         std::abs(filterFreq - lastFilterFreq) > 0.001f ||
                         pingPong != lastPingPong;

    if (valuesChanged)
    {
        // Update the UI with current parameter values
        juce::String script = "window.setDelayValues(" +
                              juce::String(delayTime) + ", " +
                              juce::String(feedback) + ", " +
                              juce::String(mix) + ", " +
                              juce::String(filterFreq) + ", " +
                              juce::String(pingPong ? "1" : "0") + ")";

        webView->evaluateJavascript(script);

        lastDelayTime = delayTime;
        lastFeedback = feedback;
        lastMix = mix;
        lastFilterFreq = filterFreq;
        lastPingPong = pingPong;
    }
}