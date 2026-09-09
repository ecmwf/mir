// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include "mir/lsm/GribFileMask.h"


namespace mir::lsm {


class GribFileMaskFromMIR : public GribFileMask {
public:
    GribFileMaskFromMIR(const std::string& name, const eckit::PathName&, const param::MIRParametrisation&,
                        const repres::Representation&, const std::string& which);

private:
    std::string name_;

    bool cacheable() const override;
    std::string cacheName() const override;
};


}  // namespace mir::lsm
