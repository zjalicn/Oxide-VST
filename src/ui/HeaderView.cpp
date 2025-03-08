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

    bool pageAboutToLoad(const juce::String &url) override
    {

        if (url.endsWith("/global.css"))
        {
            juce::Logger::writeToLog("CSS file requested");

            // Get CSS content from binary resources
            juce::MemoryOutputStream cssData;

            // Assuming the CSS file is in binary resources as "global_css"
            int size = 0;
            const char *data = BinaryData::getNamedResource("global_css", size);

            if (data != nullptr && size > 0)
            {
                juce::Logger::writeToLog("CSS found in binary resources, size: " +
                                         juce::String(size));

                // Serve the CSS content
                juce::String cssContent = juce::String(data, size);
                goToURL("data:text/css;charset=utf-8," + cssContent);
                return false; // We handled this URL
            }

            juce::Logger::writeToLog("CSS not found in binary resources");
        }

        // Let the browser handle other URLs
        return true;
    }

    std::function<void(const juce::String &)> onPresetSelected;
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
    browser->onPresetSelected = [this](const juce::String &presetName)
    {
        if (onPresetSelected)
            onPresetSelected(presetName);
    };

    browser->onSaveClicked = [this]()
    {
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

void HeaderView::paint(juce::Graphics &g)
{
    g.fillAll(juce::Colour(0xff1e1e1e));
}

void HeaderView::resized()
{
    webView->setBounds(getLocalBounds());
}