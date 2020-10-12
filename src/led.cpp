#include "led.h"

#define MAX_INT_VALUE 65536

// Configuration of initial values (for the first start) goes here....
// There's no need to change them here as they can be configured via webinterface
// Neopixel
uint8_t initialLedBrightness = 16; // Initial brightness of Neopixel
uint8_t ledBrightness = initialLedBrightness;
uint8_t nightLedBrightness = 2; // Brightness of Neopixel in nightmode

bool showLedError = false;
bool showLedOk = false;
bool showPlaylistProgress = false;
bool showRewind = false;

//Anti-aliasing code care of Mark Kriegsman Google+: https://plus.google.com/112916219338292742137/posts/2VYNQgD38Pw
void drawFractionalBar(CRGB targetStrip[], int pos16, int width, uint8_t hue, uint8_t sat, bool wrap)
{
    uint8_t stripLength = NUM_LEDS;
    uint8_t i = pos16 / 16; // convert from pos to raw pixel number

    uint8_t frac = pos16 & 0x0F; // extract the 'factional' part of the position
    uint8_t firstpixelbrightness = 255 - (frac * 16);

    uint8_t lastpixelbrightness = 255 - firstpixelbrightness;

    uint8_t bright;
    for (int n = 0; n <= width; n++)
    {
        if (n == 0)
        {
            // first pixel in the bar
            bright = firstpixelbrightness;
        }
        else if (n == width)
        {
            // last pixel in the bar
            bright = lastpixelbrightness;
        }
        else
        {
            // middle pixels
            bright = 255;
        }

        targetStrip[i] += CHSV(hue, sat, bright);
        i++;
        if (i == stripLength)
        {
            if (wrap == 1)
            {
                i = 0; // wrap around
            }
            else
            {
                return;
            }
        }
    }
}

// void xxx(CRGB leds[], uint16_t animationFrame)
// {
//     for (int i = 0; i < NUM_LEDS; i++)
//     {
//         leds[i].setRGB(0, 255, 0); // setRGB functions works by setting
//                                    // (RED value 0-255, GREEN value 0-255, BLUE value 0-255)
//                                    // RED = setRGB(255,0,0)
//                                    // GREEN = setRGB(0,255,0)
//         leds[i].fadeLightBy(animationFrame);
//     }
//     FastLED.show();
//     brightness = brightness + fadeAmount;
//     // reverse the direction of the fading at the ends of the fade:
//     if (brightness == 0 || brightness == 255)
//     {
//         fadeAmount = -fadeAmount;
//     }
//     delay(20); // This delay sets speed of the fade. I usually do from 5-75 but you can always go higher.
// }

//************************          Ring           ******************************
// Anti-aliased cyclical chaser, 3 pixels wide
// Color is determined by "hue"
//*****************************************************
void Ring(CRGB leds[], uint16_t animationFrame, uint8_t hue, uint8_t sat)
{
    uint8_t stripLength = NUM_LEDS;
    int pos16 = map(animationFrame, 0, MAX_INT_VALUE, 0, ((stripLength)*16));
    drawFractionalBar(leds, pos16, 2, hue, sat, 1);
}

void ledReadyAnimation2(CRGB leds[], uint16_t frame, uint8_t hue, uint8_t sat, uint8_t num)
{
    FastLED.clear(); //Clear previous buffer

    switch (num)
    {
    case 1:
        Ring(leds, frame, hue, sat);
        break;
    case 2:
        Ring(leds, frame, hue, sat);
        Ring(leds, frame + (MAX_INT_VALUE / 2), hue, sat);
        break;
    case 3:
        Ring(leds, frame, hue, sat);
        Ring(leds, frame + (MAX_INT_VALUE / 3), hue, sat);
        Ring(leds, frame + (MAX_INT_VALUE / 3) * 2, hue, sat);
        break;
    case 4:
        Ring(leds, frame, hue, sat);
        Ring(leds, frame + (MAX_INT_VALUE / 4), hue, sat);
        Ring(leds, frame + (MAX_INT_VALUE / 4) * 2, hue, sat);
        Ring(leds, frame + (MAX_INT_VALUE / 4) * 3, hue, sat);
        break;
    default:
        Ring(leds, frame, hue, sat);
        break;
    }

    FastLED.show(); //All animations are applied!..send the results to the strip(s)
}

// Switches Neopixel-addressing from clockwise to counter clockwise (and vice versa)
uint8_t ledAddress(uint8_t number)
{
#ifdef NEOPIXEL_REVERSE_ROTATION
    return NUM_LEDS - 1 - number;
#else
    return number;
#endif
}

void showLed(void *parameter)
{
    static uint8_t hlastVolume = currentVolume;
    static uint8_t lastPos = playProperties.currentRelPos;
    static bool lastPlayState = false;
    static bool lastLockState = false;
    static bool ledBusyShown = false;
    static bool notificationShown = false;
    static bool volumeChangeShown = false;
    static bool showEvenError = false;
    static uint8_t ledPosWebstream = 0;
    static uint8_t ledSwitchInterval = 5; // time in secs (webstream-only)
    static uint8_t webstreamColor = 0;
    static unsigned long lastSwitchTimestamp = 0;
    static bool redrawProgress = false;
    static uint8_t lastLedBrightness = ledBrightness;

    static uint16_t frame = 0; // a value from 0 to int _max

    static CRGB leds[NUM_LEDS];
    FastLED.addLeds<CHIPSET, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalSMD5050);
    FastLED.setBrightness(ledBrightness);

    for (;;)
    {

        if (gotoSleep)
        {
            FastLED.clear(true);
            FastLED.show();
            vTaskDelay(portTICK_RATE_MS * 500);
            esp_task_wdt_reset();
            continue;
        }

        if (!bootComplete)
        { // Rotates orange unless boot isn't complete
            FastLED.clear();
            for (uint8_t led = 0; led < NUM_LEDS; led++)
            {
                if (showEvenError)
                {
                    if (ledAddress(led) % 2 == 0)
                    {
                        if (millis() <= 10000)
                        {
                            leds[ledAddress(led)] = CRGB::Orange;
                        }
                        else
                        {
                            leds[ledAddress(led)] = CRGB::Red;
                        }
                    }
                }
                else
                {
                    if (millis() >= 10000)
                    { // Flashes red after 10s (will remain forever if SD cannot be mounted)
                        leds[ledAddress(led)] = CRGB::Red;
                    }
                    else
                    {
                        if (ledAddress(led) % 2 == 1)
                        {
                            leds[ledAddress(led)] = CRGB::Orange;
                        }
                    }
                }
            }
            FastLED.show();
            showEvenError = !showEvenError;
            vTaskDelay(portTICK_RATE_MS * 500);
            esp_task_wdt_reset();
            continue;
        }

        if (lastLedBrightness != ledBrightness)
        {
            FastLED.setBrightness(ledBrightness);
            lastLedBrightness = ledBrightness;
        }

#ifdef ROTARY_SWITCH_ENABLE
        if (!buttons[3].currentState)
        {
            FastLED.clear();
            for (uint8_t led = 0; led < NUM_LEDS; led++)
            {
                leds[ledAddress(led)] = CRGB::Red;
                if (buttons[3].currentState)
                {
                    FastLED.clear();
                    FastLED.show();
                    delay(5);
                    deepSleepManager();
                    break;
                }
                FastLED.show();
                vTaskDelay(intervalToLongPress / NUM_LEDS * portTICK_RATE_MS);
            }
        }
#endif

        if (showLedError)
        { // If error occured (e.g. RFID-modification not accepted)
            showLedError = false;
            notificationShown = true;
            FastLED.clear();

            for (uint8_t led = 0; led < NUM_LEDS; led++)
            {
                leds[ledAddress(led)] = CRGB::Red;
            }
            FastLED.show();
            vTaskDelay(portTICK_RATE_MS * 200);
        }

        if (showLedOk)
        { // If action was accepted
            showLedOk = false;
            notificationShown = true;
            FastLED.clear();

            for (uint8_t led = 0; led < NUM_LEDS; led++)
            {
                leds[ledAddress(led)] = CRGB::Green;
            }
            FastLED.show();
            vTaskDelay(portTICK_RATE_MS * 400);
        }

        if (hlastVolume != currentVolume)
        { // If volume has been changed
            uint8_t numLedsToLight = map(currentVolume, 0, maxVolume, 0, NUM_LEDS);
            hlastVolume = currentVolume;
            volumeChangeShown = true;
            FastLED.clear();

            for (int led = 0; led < numLedsToLight; led++)
            { // (Inverse) color-gradient from green (85) back to (still) red (245) using unsigned-cast
                leds[ledAddress(led)].setHue((uint8_t)(85 - ((double)95 / NUM_LEDS) * led));
            }
            FastLED.show();

            for (uint8_t i = 0; i <= 50; i++)
            {
                if (hlastVolume != currentVolume || showLedError || showLedOk || !buttons[3].currentState)
                {
                    if (hlastVolume != currentVolume)
                    {
                        volumeChangeShown = false;
                    }
                    break;
                }

                vTaskDelay(portTICK_RATE_MS * 20);
            }
        }

        if (showRewind)
        {
            showRewind = false;
            for (uint8_t i = NUM_LEDS - 1; i > 0; i--)
            {
                leds[ledAddress(i)] = CRGB::Black;
                FastLED.show();
                if (hlastVolume != currentVolume || lastLedBrightness != ledBrightness || showLedError || showLedOk || !buttons[3].currentState)
                {
                    break;
                }
                else
                {
                    vTaskDelay(portTICK_RATE_MS * 30);
                }
            }
        }

        if (showPlaylistProgress)
        {
            showPlaylistProgress = false;
            if (playProperties.numberOfTracks > 1 && playProperties.currentTrackNumber < playProperties.numberOfTracks)
            {
                uint8_t numLedsToLight = map(playProperties.currentTrackNumber, 0, playProperties.numberOfTracks - 1, 0, NUM_LEDS);
                FastLED.clear();
                for (uint8_t i = 0; i < numLedsToLight; i++)
                {
                    leds[ledAddress(i)] = CRGB::Blue;
                    FastLED.show();
                    if (hlastVolume != currentVolume || lastLedBrightness != ledBrightness || showLedError || showLedOk || !buttons[3].currentState)
                    {
                        break;
                    }
                    else
                    {
                        vTaskDelay(portTICK_RATE_MS * 30);
                    }
                }

                for (uint8_t i = 0; i <= 100; i++)
                {
                    if (hlastVolume != currentVolume || lastLedBrightness != ledBrightness || showLedError || showLedOk || !buttons[3].currentState)
                    {
                        break;
                    }
                    else
                    {
                        vTaskDelay(portTICK_RATE_MS * 15);
                    }
                }

                for (uint8_t i = numLedsToLight; i > 0; i--)
                {
                    leds[ledAddress(i) - 1] = CRGB::Black;
                    FastLED.show();
                    if (hlastVolume != currentVolume || lastLedBrightness != ledBrightness || showLedError || showLedOk || !buttons[3].currentState)
                    {
                        break;
                    }
                    else
                    {
                        vTaskDelay(portTICK_RATE_MS * 30);
                    }
                }
            }
        }

        switch (playProperties.playMode)
        {
        case NO_PLAYLIST: // If no playlist is active (idle)
            if (hlastVolume == currentVolume && lastLedBrightness == ledBrightness)
            {
                ledReadyAnimation2(leds, frame, 0, 0, 3);
                frame += 50;

                if (gotoSleep || hlastVolume != currentVolume || lastLedBrightness != ledBrightness ||
                    showLedError || showLedOk || playProperties.playMode != NO_PLAYLIST || !buttons[3].currentState)
                {
                    break;
                }
                vTaskDelay(portTICK_RATE_MS * 10);
            }
            break;

        case PLAYER_BUSY: // If uC is busy (parsing SD-card)
            ledBusyShown = true;

            for (uint8_t i = 0; i < 1000; i++)
            {
                ledReadyAnimation2(leds, frame, HSVHue::HUE_PURPLE, 255, 3);
                frame += 600;

                if (gotoSleep || playProperties.playMode != PLAYER_BUSY)
                {
                    break;
                }
                vTaskDelay(portTICK_RATE_MS * 10);
            }

            break;

        default: // If playlist is active (doesn't matter which type)
            if (!playProperties.playlistFinished)
            {
                if (playProperties.pausePlay != lastPlayState || lockControls != lastLockState ||
                    notificationShown || ledBusyShown || volumeChangeShown || !buttons[3].currentState)
                {
                    lastPlayState = playProperties.pausePlay;
                    lastLockState = lockControls;
                    notificationShown = false;
                    volumeChangeShown = false;
                    if (ledBusyShown)
                    {
                        ledBusyShown = false;
                        FastLED.clear();
                        FastLED.show();
                    }
                    redrawProgress = true;
                }
                if (playProperties.pausePlay)
                {
                    FastLED.clear();
                    // uint8_t numLedsToLight = map(playProperties.currentRelPos, 0, 98, 0, NUM_LEDS);
                    // for (uint8_t led = 0; led < numLedsToLight; led++) {
                    leds[ledAddress(1) % NUM_LEDS] = CRGB::Orange;
                    leds[(ledAddress(1) + NUM_LEDS / 4) % NUM_LEDS] = CRGB::Green;
                    leds[(ledAddress(1) + NUM_LEDS / 2) % NUM_LEDS] = CRGB::Orange;
                    leds[(ledAddress(1) + NUM_LEDS / 4 * 3) % NUM_LEDS] = CRGB::Green;
                    FastLED.show();
                    break;
                    // }
                }
                else if (playProperties.playMode != WEBSTREAM)
                {
                    if (playProperties.currentRelPos != lastPos || redrawProgress)
                    {
                        redrawProgress = false;
                        lastPos = playProperties.currentRelPos;
                        uint8_t numLedsToLight = map(playProperties.currentRelPos, 0, 98, 0, NUM_LEDS);
                        FastLED.clear();
                        for (uint8_t led = 0; led < numLedsToLight; led++)
                        {
                            if (lockControls)
                            {
                                leds[ledAddress(led)] = CRGB::Red;
                            }
                            else if (!playProperties.pausePlay)
                            { // Hue-rainbow
                                leds[ledAddress(led)].setHue((uint8_t)(85 - ((double)95 / NUM_LEDS) * led));
                            }
                            else if (playProperties.pausePlay)
                            {
                                leds[ledAddress(led) % NUM_LEDS] = CRGB::Orange;
                                leds[(ledAddress(led) + NUM_LEDS / 4) % NUM_LEDS] = CRGB::Orange;
                                leds[(ledAddress(led) + NUM_LEDS / 2) % NUM_LEDS] = CRGB::Orange;
                                leds[(ledAddress(led) + NUM_LEDS / 4 * 3) % NUM_LEDS] = CRGB::Orange;
                                break;
                            }
                        }
                    }
                }
                else
                { // ... but do things a little bit different for Webstream as there's no progress available
                    if (lastSwitchTimestamp == 0 || (millis() - lastSwitchTimestamp >= ledSwitchInterval * 1000) || redrawProgress)
                    {
                        redrawProgress = false;
                        lastSwitchTimestamp = millis();
                        FastLED.clear();
                        if (ledPosWebstream + 1 < NUM_LEDS)
                        {
                            ledPosWebstream++;
                        }
                        else
                        {
                            ledPosWebstream = 0;
                        }
                        if (lockControls)
                        {
                            leds[ledAddress(ledPosWebstream)] = CRGB::Red;
                            leds[(ledAddress(ledPosWebstream) + NUM_LEDS / 2) % NUM_LEDS] = CRGB::Red;
                        }
                        else if (!playProperties.pausePlay)
                        {
                            leds[ledAddress(ledPosWebstream)].setHue(webstreamColor);
                            leds[(ledAddress(ledPosWebstream) + NUM_LEDS / 2) % NUM_LEDS].setHue(webstreamColor++);
                        }
                        else if (playProperties.pausePlay)
                        {
                            leds[ledAddress(ledPosWebstream)] = CRGB::Orange;
                            leds[(ledAddress(ledPosWebstream) + NUM_LEDS / 2) % NUM_LEDS] = CRGB::Orange;
                        }
                    }
                }
                FastLED.show();
                vTaskDelay(portTICK_RATE_MS * 5);
            }
        }
        esp_task_wdt_reset();
    }
    vTaskDelete(NULL);
}
