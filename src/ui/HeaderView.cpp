#include "HeaderView.h"
#include "BinaryData.h"

class HeaderBrowserComponent : public juce::WebBrowserComponent
{
public:
    HeaderBrowserComponent()
    {
        onPresetSelected = nullptr;
        onSaveClicked = nullptr;
    }

    bool pageAboutToLoad(const juce::String &url) override
    {
        // Handle oxide: protocol for control messages
        if (url.startsWith("oxide:"))
        {
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

            return false; // We handled this URL
        }

        return true; // We didn't handle this URL
    }

    // Callback for when a preset is selected
    std::function<void(const juce::String &)> onPresetSelected;

    // Callback for when save is clicked
    std::function<void()> onSaveClicked;
};

HeaderView::HeaderView()
{
    // Create the browser with the resource handler
    auto browser = new HeaderBrowserComponent();
    webView.reset(browser);
    webView->setOpaque(false);
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

    // Get the HTML content
    juce::String htmlContent = juce::String(BinaryData::header_html, BinaryData::header_htmlSize);

    // Inject CSS directly into HTML head
    juce::String cssContent = juce::String(BinaryData::header_css, BinaryData::header_cssSize);
    htmlContent = htmlContent.replace(
        "<link rel=\"stylesheet\" href=\"./global.css\" />",
        "<style>" + cssContent + "</style>");

    // Load the combined HTML content
    webView->goToURL("data:text/html;charset=utf-8," + htmlContent);
}

HeaderView::~HeaderView()
{
    webView = nullptr;
}

void HeaderView::paint(juce::Graphics &g)
{
    g.fillAll(juce::Colour(0xff1e1e1e));
}

void HeaderView::resized()
{
    webView->setBounds(getLocalBounds());
}