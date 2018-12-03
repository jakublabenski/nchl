
#include "colors.h"

#include <Arduino.h>

#include <algorithm>
#include <cmath>

namespace
{

struct Color
{
    Color(uint8_t r, uint8_t g, uint8_t b) : r(r), g(g), b(b) {}

    uint32_t toColor(double brightness) const;

    uint8_t r = 0;
    uint8_t g = 0;
    uint8_t b = 0;
};

using ColorVector = std::vector<Color>;

void setUpColors(Colors &colors, const ColorVector &set, double brightness)
{
    int idx = 0;
    for (auto &element : colors)
    {
        element = set[idx % set.size()].toColor(brightness);
        ++idx;
    }
}

bool handleYellowFlicker(Colors& out_colors, double brightness)
{
    static unsigned long last_update = 0;

    unsigned long loop_time = millis();
    double intensity = 0.5;

    if (loop_time - last_update > 1000) {
        ColorVector c;
        for(size_t i = 0; i < out_colors.size(); ++i) {
            int g = 20 + random(88, 99) * std::pow(intensity, 2);
            int r = std::min(random(75, 100) * std::pow(intensity + 0.1, 0.75), 100.0);
            c.emplace_back(
                r, //std::min(random(75, 100) * std::pow(intensity + 0.1, 0.75), 100.0),
                g, //random(88, 99) * std::pow(intensity, 2),
                0);
        }

//                std::min(random(75, 100) * std::pow(intensity + 0.1, 0.75), 100.0),
//                random(33, 44) * std::pow(intensity, 2),


        setUpColors(out_colors, c, brightness);

        last_update = loop_time;

        return true;
    }
    return false;
}

// Input a value 0 to 255 to get a color value.
// The colours are a transition r - g - b - back to r.
Color wheel(byte WheelPos)
{
    WheelPos = 255 - WheelPos;
    if (WheelPos < 85) {
        return Color(255 - WheelPos * 3, 0, WheelPos * 3);
    }
    if (WheelPos < 170) {
        WheelPos -= 85;
        return Color(0, WheelPos * 3, 255 - WheelPos * 3);
    }
    WheelPos -= 170;
    return Color(WheelPos * 3, 255 - WheelPos * 3, 0);
}

void rainbowCycle(Colors& out_colors, double brightness, uint16_t cycle)
{
    for (uint16_t i = 0; i < out_colors.size(); i++) {
        auto new_color =  wheel(((i * 256 / out_colors.size()) + cycle) & 255);
        out_colors[i] = new_color.toColor(brightness);
    }
}

bool handleRanibow(Colors& out_colors, double brightness)
{
    static unsigned long last_update = 0;
    static uint16_t cycle = 0;

    unsigned long loop_time = millis();

    if (loop_time - last_update > 20) {
        rainbowCycle(out_colors, brightness, cycle);
        cycle = (cycle + 1) % 256;
        last_update = loop_time;

        return true;
    }

    return false;
}

bool simpleColorHandler(Colors& out_colors, const ColorVector& colors,
    double brightness)
{
    static unsigned long last_update = 0;

    unsigned long loop_time = millis();

    if (loop_time - last_update > 200) {
        setUpColors(out_colors, colors, brightness);

        last_update = loop_time;

        return true;
    }
    return false;
}

} // namespace

uint32_t Color::toColor(double brightness) const
{
    return ((uint32_t)(brightness * r) << 16) 
        | ((uint32_t)(brightness * g) << 8)
        | (uint32_t)(brightness * b);
}

bool colors(Colors &out_colors, const Data &data)
{

    switch (data.mode())
    {
    case Mode::RGBY:
    {
        ColorVector rgb = {
            {255, 0, 0},
            {0, 255, 0},
            {0, 0, 255},
            {255, 255, 0},
        };
        return simpleColorHandler(out_colors, rgb, data.brightness());
    }
    case Mode::RGOBY:
    {
        ColorVector rgb = {
            {255, 0, 0},
            {0, 255, 0},
            {255, 165, 0},
            {0, 0, 255},
            {255, 255, 0},
        };
        return simpleColorHandler(out_colors, rgb, data.brightness());
    }
    case Mode::YELLOW:
    {
        ColorVector c = {
            {248, 222, 126},
            {250, 218, 94},
            {249, 166, 2},
            {255, 211, 0},
            {252, 244, 163},
            {252, 226, 5},
            {255, 253, 208},
            {239, 253, 95},
        };
        return simpleColorHandler(out_colors, c, data.brightness());
    }
    case Mode::GREEN:
    {
        ColorVector c = {
            {100, 250, 0},
            {0, 255, 0},
            {0, 255, 0},
            {0, 200, 0},
            {20, 200, 20},
        };
        return simpleColorHandler(out_colors, c, data.brightness());
    }
    case Mode::RAINBOW:
        return handleRanibow(out_colors, data.brightness());
    case Mode::FLICKER_YELLOW:
        return handleYellowFlicker(out_colors, data.brightness());

    }

    return false;
}