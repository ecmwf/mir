// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include <string>
#include <vector>


namespace mir::appendix {


std::vector<double> grid_box_areas(const std::string& gridspec);


}  // namespace mir::appendix
