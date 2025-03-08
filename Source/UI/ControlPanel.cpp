#include "ControlPanel.h"
#include "BinaryData.h"

class OxideWebBrowser : public juce::WebBrowserComponent
{
public:
    OxideWebBrowser(DistortionProcessor& proc) : processor(proc) {}
    
    bool pageAboutToLoad(const juce::String& url) override
    {
        juce::Logger::writeToLog("URL: " + url);
        
        // If URL starts with "oxide:", it's a control message
        if (url.startsWith("oxide:"))
        {
            // Parse the URL to get parameters
            juce::String params = url.fromFirstOccurrenceOf("oxide:", false, true);
            
            if (params.startsWith("drive="))
            {
                float value = params.fromFirstOccurrenceOf("drive=", false, true).getFloatValue();
                juce::Logger::writeToLog("Setting drive to: " + juce::String(value));
                processor.setDrive(value);
                return false;
            }
            else if (params.startsWith("mix="))
            {
                float value = params.fromFirstOccurrenceOf("mix=", false, true).getFloatValue();
                juce::Logger::writeToLog("Setting mix to: " + juce::String(value));
                processor.setMix(value);
                return false;
            }
            
            // Don't actually navigate to the URL
            return false;
        }
        
        // Allow normal navigation
        return true;
    }
    
private:
    DistortionProcessor& processor;
};

ControlPanel::ControlPanel(DistortionProcessor& processor)
    : distortionProcessor(processor),
      lastDrive(processor.getDrive()),
      lastMix(processor.getMix())
{
    webView.reset(new OxideWebBrowser(distortionProcessor));
    webView->setWantsKeyboardFocus(false);
    webView->setFocusContainer(false);
    addAndMakeVisible(webView.get());
    
    juce::String htmlContent = juce::String(BinaryData::controlPanel_html, BinaryData::controlPanel_htmlSize);
    webView->goToURL("data:text/html;charset=utf-8," + htmlContent);
    
    startTimer(50);
}

ControlPanel::~ControlPanel()
{
    stopTimer();
}

void ControlPanel::paint(juce::Graphics& g)
{
}

void ControlPanel::resized()
{
    webView->setBounds(getLocalBounds());
}

void ControlPanel::timerCallback()
{
    float drive = distortionProcessor.getDrive();
    float mix = distortionProcessor.getMix();
    
    if (std::abs(drive - lastDrive) > 0.001f || std::abs(mix - lastMix) > 0.001f)
    {
        // Use the simpler direct method
        juce::String script = "window.setValues(" + juce::String(drive) + ", " + juce::String(mix) + ")";
        webView->evaluateJavascript(script);
        
        lastDrive = drive;
        lastMix = mix;
    }
}