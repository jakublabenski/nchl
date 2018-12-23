
#include "data.h"

#include <Arduino.h>

#include "json11.hpp"

#include <ctype.h>
#include <time.h>

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

bool parse_mode(const std::string &new_mode, Mode &out_mode)
{
    if (new_mode.size() == 0)
    {
        Serial.println("Error: empty value of 'type' field");
        return false;
    }

    switch ((Mode)new_mode[0])
    {
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
    case Mode::RGBY:
        out_mode = Mode::RGBY;
        return true;
    case Mode::RGOBY:
        out_mode = Mode::RGOBY;
        return true;
    case Mode::WHITE:
        out_mode = Mode::WHITE;
        return true;
    case Mode::RED:
        out_mode = Mode::RED;
        return true;
    case Mode::BLUE:
        out_mode = Mode::BLUE;
        return true;
     case Mode::DISABLE:
        out_mode = Mode::DISABLE;
        return true;
    }
    return false;
}

bool parse_time(const std::string &str, int &total_minutes)
{
    if (str.size() != 5)
    {
        return false;
    }

    if (!isdigit(str[0]))
    {
        return false;
    }
    if (!isdigit(str[1]))
    {
        return false;
    }
    if (str[2] != ':')
    {
        return false;
    }
    if (!isdigit(str[3]))
    {
        return false;
    }
    if (!isdigit(str[4]))
    {
        return false;
    }

    int hours = 10 * (str[0] - '0') + (str[1] - '0');
    if (hours < 0 || hours > 23)
    {
        return false;
    }
    int minutes = 10 * (str[3] - '0') + (str[4] - '0');
    if (minutes < 0 || minutes > 59)
    {
        return false;
    }

    total_minutes = 60 * hours + minutes;
    return true;
}

} // namespace

bool Data::enabled(int hours, int minutes) const
{
    if (!timer_)
    {
        return true;
    }

    int current = 60 * hours + minutes;

    int start;

    if (!parse_time(start_time_, start))
    {
        return true;
    }

    int stop;
    if (!parse_time(stop_time_, stop))
    {
        return true;
    }

    return (start < stop)
               ? (start <= current && current < stop)
               : !(start > current && current >= stop);
}

bool Data::enabled_now() const
{
    if (!timer_)
    {
        return true;
    }
    time_t now = time(nullptr);
    struct tm *t = localtime(&now);

    return enabled(t->tm_hour, t->tm_min);
}

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
    if (!read_string(object, "type", new_type))
    {
        return;
    }

    int new_brightness = 0.0;
    if (!read_number(object, "brightness", new_brightness))
    {
        return;
    }

    bool new_timer = false;
    if (!read_bool(object, "timer", new_timer))
    {
        return;
    }

    std::string new_start_time;
    if (!read_string(object, "start_time", new_start_time))
    {
        return;
    }

    std::string new_stop_time;
    if (!read_string(object, "stop_time", new_stop_time))
    {
        return;
    }

    if (!parse_mode(new_type, mode_))
    {
        return;
    }

    int new_number_of_leds = 0.0;
    if (!read_number(object, "number_of_leds", new_number_of_leds))
    {
        return;
    }

    int new_change_delay = 0.0;
    if (!read_number(object, "change_delay", new_change_delay))
    {
        return;
    }

    brightness_ = new_brightness;
    timer_ = new_timer;
    start_time_ = new_start_time;
    stop_time_ = new_stop_time;
    number_of_leds_ = new_number_of_leds;
    change_dalay_ = new_change_delay;
}

std::string Data::to_string(bool as_keys) const
{
    char buf[max_data_size];
    snprintf(buf, max_data_size,
             "\"brightness\": %u, \"type\": \"%c\", \"timer\": %s,"
             "\"start_time\": \"%s\", \"stop_time\": \"%s\","
             "\"number_of_leds\": %d, "
             "\"change_delay\": %d",
             brightness_, mode_,
             timer_ ? "true" : "false",
             start_time_.c_str(),
             stop_time_.c_str(),
             number_of_leds_,
             change_dalay_);

    std::string ret(buf);
    return as_keys ? ret : ("{" + ret + "}");
}