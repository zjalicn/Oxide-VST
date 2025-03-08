#include "HeaderView.h"
#include "BinaryData.h"

class HeaderWebBrowser : public juce::WebBrowserComponent
{
public:
    HeaderWebBrowser() 
    {
        onPresetSelected = nullptr;
        onSaveClicked = nullptr;
    }
    
    bool pageAboutToLoad(const juce::String& url) override
    {
        // If URL starts with "oxide:", it's a control message
        if (url.startsWith("oxide:"))
        {
            // Parse the URL to get parameters
            juce::String params = url.fromFirstOccurrenceOf("oxide:", false, true);
            
            if (params.startsWith("preset="))
            {
                juce::String presetName = params.fromFirstOccurrenceOf("preset=", false, true);
                if (onPresetSelected)
                    onPresetSelected(presetName);
                return false;
            }
            else if (params.startsWith("action=save"))
            {
                if (onSaveClicked)
                    onSaveClicked();
                return false;
            }
            
            // Don't actually navigate to the URL
            return false;
        }
        
        // Allow normal navigation
        return true;
    }
    
    std::function<void(const juce::String&)> onPresetSelected;
    std::function<void()> onSaveClicked;
};

HeaderView::HeaderView()
{
    // Create the web view - disable keyboard focus
    auto browser = new HeaderWebBrowser();
    webView.reset(browser);
    webView->setWantsKeyboardFocus(false);
    webView->setFocusContainer(false);
    addAndMakeVisible(webView.get());
    
    // Set up callbacks
    browser->onPresetSelected = [this](const juce::String& presetName) {
        if (onPresetSelected)
            onPresetSelected(presetName);
    };
    
    browser->onSaveClicked = [this]() {
        if (onSaveClicked)
            onSaveClicked();
    };
    
    juce::String htmlContent = juce::String(BinaryData::header_html, BinaryData::header_htmlSize);
    webView->goToURL("data:text/html;charset=utf-8," + htmlContent);
}

HeaderView::~HeaderView()
{
    webView->setOpaque(false);
}

void HeaderView::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colour(0xff1e1e1e));
}

void HeaderView::resized()
{
    webView->setBounds(getLocalBounds());
}