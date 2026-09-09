// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/lsm/GribFileMaskFromUser.h"


namespace mir::lsm {


bool GribFileMaskFromUser::cacheable() const {
    return false;
}


std::string GribFileMaskFromUser::cacheName() const {
    return path_;
}


}  // namespace mir::lsm
