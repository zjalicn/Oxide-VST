#include "MeterView.h"

MeterView::MeterView()
{
    // Set up the web view
    webView.reset(new juce::WebBrowserComponent(false));
    addAndMakeVisible(webView.get());
    
    createHtmlContent();
}

MeterView::~MeterView()
{
    // Clean up the temporary HTML file
    if (tempHtmlFile.existsAsFile())
        tempHtmlFile.deleteFile();
}

void MeterView::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::black);
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
    juce::String script = juce::String("setAudioLevels(") + 
                          juce::String(leftLevel) + 
                          juce::String(", ") + 
                          juce::String(rightLevel) + 
                          juce::String(")");
    
    webView->executeJavaScript(script, nullptr);
}

void MeterView::createHtmlContent()
{
    // HTML & CSS for the meter display
    juce::String htmlContent = R"(
        <!DOCTYPE html>
        <html>
        <head>
            <style>
                body {
                    margin: 0;
                    padding: 0;
                    background-color: #222;
                    font-family: Arial, sans-serif;
                    color: white;
                    overflow: hidden;
                }
                
                .container {
                    display: flex;
                    flex-direction: column;
                    align-items: center;
                    justify-content: center;
                    height: 100vh;
                    padding: 20px;
                }
                
                h1 {
                    margin-bottom: 20px;
                    text-align: center;
                    color: #ff6600;
                    font-size: 24px;
                }
                
                .meters-container {
                    display: flex;
                    justify-content: center;
                    width: 100%;
                    max-width: 600px;
                }
                
                .meter {
                    display: flex;
                    flex-direction: column;
                    align-items: center;
                    margin: 0 20px;
                    width: 60px;
                }
                
                .meter-label {
                    margin-bottom: 10px;
                    font-size: 16px;
                }
                
                .meter-bar-container {
                    height: 300px;
                    width: 40px;
                    background-color: #333;
                    border-radius: 5px;
                    position: relative;
                    overflow: hidden;
                    border: 1px solid #444;
                }
                
                .meter-bar {
                    position: absolute;
                    bottom: 0;
                    width: 100%;
                    background: linear-gradient(to top, #0f0, #ff0, #f00);
                    transition: height 0.1s ease;
                }
                
                .meter-value {
                    margin-top: 10px;
                    font-size: 14px;
                }
                
                .meter-scale {
                    position: absolute;
                    width: 100%;
                    height: 1px;
                    background-color: rgba(255, 255, 255, 0.2);
                }
                
                .meter-scale-label {
                    position: absolute;
                    right: -30px;
                    transform: translateY(-50%);
                    font-size: 10px;
                    color: rgba(255, 255, 255, 0.6);
                }
            </style>
        </head>
        <body>
            <div class="container">
                <h1>OXIDE</h1>
                <div class="meters-container">
                    <div class="meter">
                        <div class="meter-label">Left</div>
                        <div class="meter-bar-container">
                            <div id="leftMeter" class="meter-bar" style="height: 0%"></div>
                            <!-- Level markers -->
                            <div class="meter-scale" style="bottom: 70%;">
                                <span class="meter-scale-label">-3dB</span>
                            </div>
                            <div class="meter-scale" style="bottom: 50%;">
                                <span class="meter-scale-label">-6dB</span>
                            </div>
                            <div class="meter-scale" style="bottom: 25%;">
                                <span class="meter-scale-label">-12dB</span>
                            </div>
                        </div>
                        <div id="leftValue" class="meter-value">-∞ dB</div>
                    </div>
                    <div class="meter">
                        <div class="meter-label">Right</div>
                        <div class="meter-bar-container">
                            <div id="rightMeter" class="meter-bar" style="height: 0%"></div>
                            <!-- Level markers -->
                            <div class="meter-scale" style="bottom: 70%;">
                                <span class="meter-scale-label">-3dB</span>
                            </div>
                            <div class="meter-scale" style="bottom: 50%;">
                                <span class="meter-scale-label">-6dB</span>
                            </div>
                            <div class="meter-scale" style="bottom: 25%;">
                                <span class="meter-scale-label">-12dB</span>
                            </div>
                        </div>
                        <div id="rightValue" class="meter-value">-∞ dB</div>
                    </div>
                </div>
            </div>
            <script>
                function updateMeters(left, right) {
                    const leftMeter = document.getElementById('leftMeter');
                    const rightMeter = document.getElementById('rightMeter');
                    const leftValue = document.getElementById('leftValue');
                    const rightValue = document.getElementById('rightValue');
                    
                    // Update meter heights (0-100%)
                    leftMeter.style.height = left + '%';
                    rightMeter.style.height = right + '%';
                    
                    // Calculate dB values (approximation)
                    const leftDb = left > 0 ? (20 * Math.log10(left / 100)) : -100;
                    const rightDb = right > 0 ? (20 * Math.log10(right / 100)) : -100;
                    
                    leftValue.textContent = leftDb > -50 ? leftDb.toFixed(1) + ' dB' : '-∞ dB';
                    rightValue.textContent = rightDb > -50 ? rightDb.toFixed(1) + ' dB' : '-∞ dB';
                }
                
                // This will be called from JUCE
                window.setAudioLevels = updateMeters;
            </script>
        </body>
        </html>
    )";
    
    tempHtmlFile = juce::File::createTempFile("oxide_meter.html");
    tempHtmlFile.replaceWithText(htmlContent);
    webView->goToURL(tempHtmlFile.getFullPathName());
}