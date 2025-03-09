#include "HeaderView.h"
#include "BinaryData.h"
#include "CustomFonts.h"

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
    // Load the custom font to make sure it's available to the system
    auto font = CustomFonts::getOldEnglishFont();

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

    // Get the CSS content
    juce::String cssContent = juce::String(BinaryData::header_css, BinaryData::header_cssSize);

    // Get the font data
    int fontDataSize = 0;
    const char *fontData = BinaryData::getNamedResource("old_english_hearts_ttf", fontDataSize);

    // Create the font CSS
    juce::String fontCSS;

    if (fontData != nullptr && fontDataSize > 0)
    {
        // Convert to Base64
        juce::MemoryBlock fontMemBlock(fontData, fontDataSize);
        juce::String fontBase64 = fontMemBlock.toBase64Encoding();

        // Create font-face CSS
        fontCSS =
            "@font-face {\n"
            "  font-family: 'OldEnglishHearts';\n"
            "  src: url('data:application/x-font-ttf;base64," +
            fontBase64 + "') format('truetype');\n"
                         "  font-weight: normal;\n"
                         "  font-style: normal;\n"
                         "}\n\n"
                         ".font-special {\n"
                         "  font-family: 'OldEnglishHearts', 'Lucida Grande', Arial, sans-serif;\n"
                         "}\n";

        juce::Logger::writeToLog("Font embedded in CSS with " + juce::String(fontBase64.length()) + " characters");
    }
    else
    {
        // Fallback
        fontCSS = ".font-special { font-weight: bold; }\n";
        juce::Logger::writeToLog("ERROR: Font binary data not found");
    }

    // Combine CSS and inject into HTML
    cssContent = fontCSS + cssContent;
    htmlContent = htmlContent.replace(
        "<link rel=\"stylesheet\" href=\"./global.css\" />",
        "<style>\n" + cssContent + "\n</style>");

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