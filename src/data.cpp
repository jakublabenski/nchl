
#include "data.h"

#include <Arduino.h>

#include "json11.hpp"

namespace
{
bool read_string(const json11::Json::object &object,
                 const std::string &name, std::string &out)
{
    const auto i = object.find(name);
    if (i == object.end())
    {
        Serial.printf("Error: missing '%s' value\n", name.c_str());
        return false;
    }

    const auto &value = i->second;
    if (!value.is_string())
    {
        Serial.printf("Error: type of '%s' field should be string\n",
                      name.c_str());
        return false;
    }
    out = value.string_value();
    return true;
}

bool read_number(const json11::Json::object &object,
                 const std::string &name, int &out)
{
    const auto i = object.find(name);
    if (i == object.end())
    {
        Serial.printf("Error: missing '%s' value\n", name.c_str());
        return false;
    }

    const auto &value = i->second;
    if (!value.is_number())
    {
        Serial.printf("Error: type of '%s' field should be number\n",
                      name.c_str());
        return false;
    }
    out = value.int_value();
    
    return true;
}

bool read_bool(const json11::Json::object &object,
                 const std::string &name, bool &out)
{
    const auto i = object.find(name);
    if (i == object.end())
    {
        Serial.printf("Error: missing '%s' value\n", name.c_str());
        return false;
    }

    const auto &value = i->second;
    if (!value.is_bool())
    {
        Serial.printf("Error: type of '%s' field should be bool\n",
                      name.c_str());
        return false;
    }
    out = value.bool_value();
    
    return true;
}

bool parse_mode(const std::string& new_mode, Mode& out_mode)
{
    if (new_mode.size() == 0)
    {
        Serial.println("Error: empty value of 'type' field");
        return false;
    }

    switch((Mode)new_mode[0]) {
        case Mode::RAINBOW:
            out_mode = Mode::RAINBOW;
            return true;
        case Mode::GREEN:
            out_mode = Mode::GREEN;
            return true;
        case Mode::YELLOW:
            out_mode = Mode::YELLOW;
            return true;
        case Mode::FLICKER_YELLOW:
            out_mode = Mode::FLICKER_YELLOW;
            return true;
        case Mode::RGB:
            out_mode = Mode::RGB;
            return true;
    }
    return false;
}

} // namespace

void Data::from_string(const std::string &p)
{
    std::string error;

    json11::Json json = json11::Json::parse(p, error);

    if (!error.empty())
    {
        Serial.print("Error: parsing json failed:");
        Serial.println(error.c_str());
        return;
    }

    if (!json.is_object())
    {
        Serial.println("Error: expected json object");
        return;
    }

    auto &object = json.object_items();

    std::string new_type;
    if (!read_string(object, "type", new_type)) {
        return;
    }

    int new_brightness = 0.0;
    if (!read_number(object, "brightness", new_brightness)) {
        return;
    }

    bool new_timer = false;
    if (!read_bool(object, "timer", new_timer)) {
        return;
    }

    std::string new_start_time;
    if (!read_string(object, "start_time", new_start_time)) {
        return;
    }

    std::string new_stop_time;
    if (!read_string(object, "stop_time", new_stop_time)) {
        return;
    }

    if (!parse_mode(new_type, mode_)) {
        return;
    }


    brightness_ = new_brightness;
    timer_ = new_timer;
    start_time_ = new_start_time;
    stop_time_ = new_stop_time;
}

std::string Data::to_string(bool as_keys) const
{
    const int buf_size = 132;
    char buf[buf_size];
    snprintf(buf, buf_size,
             "\"brightness\": %u, \"type\": \"%c\", \"timer\": %s,"
             "\"start_time\": \"%s\", \"stop_time\": \"%s\"",
             brightness_, mode_,
             timer_ ? "true" : "false",
             start_time_.c_str(),
             stop_time_.c_str());

    std::string ret(buf);
    return as_keys ? ret : ("{" + ret + "}");
}