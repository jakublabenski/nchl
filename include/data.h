
#ifndef __DATA_H__
#define __DATA_H__

#include <string>

enum class Mode : unsigned char
{
    RAINBOW = 'R',
    RGB = 'B',
    GREEN = 'G',
    YELLOW = 'Y',
    FLICKER_YELLOW = 'F',
};

class Data
{
  public:
    Mode mode() const
    {
        return mode_;
    }

    double brightness() const
    {
        return brightness_ / 1024.0;
    }

    void from_string(const std::string &);
    std::string to_string(bool as_keys) const;

  private:
    std::string start_time_;
    std::string stop_time_;
    uint32_t brightness_ = 512U;
    Mode mode_ = Mode::RAINBOW;
    bool timer_ = false;
};

#endif