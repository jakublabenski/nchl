
#ifndef __COLORS_H__
#define __COLORS_H__

#include "data.h"

#include <vector>

using Colors = std::vector<uint32_t>;

bool colors(Colors& out_colors, const Data& data);

#endif