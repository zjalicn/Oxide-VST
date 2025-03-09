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

            return false;
        }

        return true;
    }

    std::function<void(const juce::String &)> onPresetSelected;
    std::function<void()> onSaveClicked;
};

HeaderView::HeaderView()
{
    auto browser = new HeaderBrowserComponent();
    webView.reset(browser);
    webView->setOpaque(false);
    webView->setWantsKeyboardFocus(false);
    webView->setFocusContainer(false);
    addAndMakeVisible(webView.get());

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

    // Get the CSS content
    juce::String cssContent = juce::String(BinaryData::header_css, BinaryData::header_cssSize);

    // Inject CSS into HTML
    htmlContent = htmlContent.replace(
        "<link rel=\"stylesheet\" href=\"./global.css\" />",
        "<style>\n" + cssContent + "\n</style>");

    // Load the HTML content
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