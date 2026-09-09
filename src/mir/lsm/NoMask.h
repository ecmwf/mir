// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include "mir/lsm/Mask.h"


namespace mir::lsm {


class NoMask : public Mask {

    bool active() const override;

    bool cacheable() const override;

    void hash(eckit::MD5&) const override;

    const std::vector<bool>& mask() const override;

    void print(std::ostream&) const override;

    std::string cacheName() const override;
};


}  // namespace mir::lsm
