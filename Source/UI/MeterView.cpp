#include "MeterView.h"
#include "BinaryData.h"

MeterView::MeterView()
{
    // Create the web view
    webView.reset(new juce::WebBrowserComponent());
    addAndMakeVisible(webView.get());
    
    // Load HTML from binary resources
    juce::String htmlContent = juce::String(BinaryData::meter_html, BinaryData::meter_htmlSize);
    webView->goToURL("data:text/html;charset=utf-8," + htmlContent);
}

MeterView::~MeterView()
{
}

void MeterView::paint(juce::Graphics& g)
{
    // No additional painting needed
}

void MeterView::resized()
{
    webView->setBounds(getLocalBounds());
}

void MeterView::updateLevels(float leftLevel, float rightLevel)
{
    // Limit and scale the levels
    leftLevel = juce::jlimit(0.0f, 100.0f, leftLevel);
    rightLevel = juce::jlimit(0.0f, 100.0f, rightLevel);
    
    // Send the levels to the web view
    juce::String script = "setAudioLevels(" + juce::String(leftLevel) + ", " + juce::String(rightLevel) + ")";
    webView->evaluateJavascript(script);
}