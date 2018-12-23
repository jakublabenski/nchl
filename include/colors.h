
#ifndef __COLORS_H__
#define __COLORS_H__

#include "data.h"

#include <vector>

using Colors = std::vector<uint32_t>;

bool set_colors(Colors& out_colors, Mode mode);

bool update_colors(Colors& out_colors, Mode mode);

#endif