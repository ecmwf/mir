// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include "mir/lsm/GribFileMask.h"


namespace mir::lsm {


class GribFileMaskFromUser : public GribFileMask {

    bool cacheable() const override;
    std::string cacheName() const override;

public:
    using GribFileMask::GribFileMask;
};


}  // namespace mir::lsm
