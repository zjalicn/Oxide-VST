#include "ControlPanel.h"
#include "BinaryData.h"

class ControlPanelBrowserComponent : public juce::WebBrowserComponent
{
public:
    ControlPanelBrowserComponent(DistortionProcessor &proc)
        : processor(proc) {}

    bool pageAboutToLoad(const juce::String &url) override
    {
        // Handle oxide: protocol for control messages
        if (url.startsWith("oxide:"))
        {
            juce::String params = url.fromFirstOccurrenceOf("oxide:", false, true);

            if (params.startsWith("drive="))
            {
                float value = params.fromFirstOccurrenceOf("drive=", false, true).getFloatValue();
                processor.setDrive(value);
                return false;
            }
            else if (params.startsWith("mix="))
            {
                float value = params.fromFirstOccurrenceOf("mix=", false, true).getFloatValue();
                processor.setMix(value);
                return false;
            }
            else if (params.startsWith("algorithm="))
            {
                juce::String value = params.fromFirstOccurrenceOf("algorithm=", false, true);
                processor.setAlgorithm(value);
                return false;
            }

            return false; // We handled this URL
        }

        return true; // We didn't handle this URL
    }

private:
    DistortionProcessor &processor;
};

ControlPanel::ControlPanel(DistortionProcessor &processor)
    : distortionProcessor(processor),
      lastDrive(processor.getDrive()),
      lastMix(processor.getMix()),
      lastAlgorithm(processor.getAlgorithmName())
{
    // Create the browser with the resource handler
    auto browser = new ControlPanelBrowserComponent(distortionProcessor);
    webView.reset(browser);
    webView->setFocusContainer(false);
    addAndMakeVisible(webView.get());

    // Get the HTML content
    juce::String htmlContent = juce::String(BinaryData::controlPanel_html, BinaryData::controlPanel_htmlSize);

    // Inject CSS directly into HTML head
    juce::String cssContent = juce::String(BinaryData::controlpanel_css, BinaryData::controlpanel_cssSize);

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

ControlPanel::~ControlPanel()
{
    webView = nullptr;
    stopTimer();
}

void ControlPanel::paint(juce::Graphics &g)
{
    // Nothing to paint here - WebBrowserComponent handles rendering
}

void ControlPanel::resized()
{
    // Make sure the webView takes up the exact same area as this component
    webView->setBounds(getLocalBounds());
}

void ControlPanel::timerCallback()
{
    float drive = distortionProcessor.getDrive();
    float mix = distortionProcessor.getMix();
    juce::String algorithm = distortionProcessor.getAlgorithmName();

    bool valuesChanged = std::abs(drive - lastDrive) > 0.001f ||
                         std::abs(mix - lastMix) > 0.001f ||
                         algorithm != lastAlgorithm;

    if (valuesChanged)
    {
        // Update the UI with current parameter values
        juce::String script = "window.setValues(" +
                              juce::String(drive) + ", " +
                              juce::String(mix) + ", '" +
                              algorithm + "')";

        webView->evaluateJavascript(script);

        lastDrive = drive;
        lastMix = mix;
        lastAlgorithm = algorithm;
    }
}