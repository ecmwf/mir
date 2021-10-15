/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 *
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include "mir/netcdf/PackingCodec.h"

#include <netcdf.h>

#include <algorithm>
#include <ostream>
#include <sstream>

#include "mir/netcdf/Exceptions.h"
#include "mir/netcdf/OutputAttribute.h"
#include "mir/netcdf/Value.h"
#include "mir/netcdf/Variable.h"


namespace mir {
namespace netcdf {


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


static CodecBuilder<PackingCodec> builder("packing");


}  // namespace netcdf
}  // namespace mir
