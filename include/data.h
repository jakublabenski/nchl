
#ifndef __DATA_H__
#define __DATA_H__

#include <string>

enum class Mode : unsigned char
{
    RAINBOW = 'R',
    RGBY = 'B',
    RGOBY = 'O',
    GREEN = 'G',
    YELLOW = 'Y',
    FLICKER_YELLOW = 'F',
    WHITE = 'W',
    BLUE = 'L',
    RED = 'D',
    DISABLE = 'X'
};

class Data
{
  public:
    static const int max_data_size = 256;

    Mode mode() const
    {
        return mode_;
    }

    uint8_t brightness() const
    {
        return brightness_ / 4;
    }

    bool timer() const
    {
        return timer_;
    }

    bool enabled(int hours, int minutes) const;

    bool enabled_now() const;

    void from_string(const std::string &);
    std::string to_string(bool as_keys) const;

    uint16_t number_of_leds() const
    {
        return number_of_leds_;
    }

    uint16_t change_dalay() const
    {
        return change_dalay_;
    }
  private:
    std::string start_time_;
    std::string stop_time_;
    uint32_t brightness_ = 512U;
    uint16_t number_of_leds_ = 50;
    uint16_t change_dalay_ = 20;
    Mode mode_ = Mode::RAINBOW;
    bool timer_ = false;
    bool enabled_ = true;
};

#endif