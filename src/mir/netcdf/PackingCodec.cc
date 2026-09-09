// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/netcdf/PackingCodec.h"

#include <netcdf.h>

#include <algorithm>
#include <ostream>
#include <sstream>

#include "mir/netcdf/Exceptions.h"
#include "mir/netcdf/OutputAttribute.h"
#include "mir/netcdf/Value.h"
#include "mir/netcdf/Variable.h"


namespace mir::netcdf {


PackingCodec::PackingCodec(const Variable& variable) :
    scale_factor_(variable.getAttributeValue<double>("scale_factor")),
    add_offset_(variable.hasAttribute("add_offset") ? variable.getAttributeValue<double>("add_offset") : 0.) {}


PackingCodec::~PackingCodec() = default;


void PackingCodec::print(std::ostream& out) const {
    out << "PackingCodec[scale_factor=" << scale_factor_ << ", add_offset=" << add_offset_ << "]";
}


void PackingCodec::decode(std::vector<double>& v) const {
    for (auto& i : v) {
        i = i * scale_factor_ + add_offset_;
    }
}


static const CodecBuilder<PackingCodec> builder("packing");


}  // namespace mir::netcdf
