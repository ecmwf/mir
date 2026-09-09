// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include "mir/netcdf/Codec.h"


namespace mir::netcdf {


class PackingCodec : public Codec {
public:
    PackingCodec(const Variable&);
    ~PackingCodec() override;

private:
    double scale_factor_;
    double add_offset_;

    // -- Methods
    void print(std::ostream&) const override;
    void decode(std::vector<double>&) const override;
};


}  // namespace mir::netcdf
