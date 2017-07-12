/*
 * (C) Copyright 1996-2013 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

// Baudouin Raoult - ECMWF Jan 2015

#include "mir/netcdf/PackingCodec.h"

#include "mir/netcdf/Exceptions.h"
#include "mir/netcdf/OutputAttribute.h"
#include "mir/netcdf/Value.h"
#include "mir/netcdf/Variable.h"

#include <ostream>
#include <sstream>
#include <algorithm>

#include <netcdf.h>

namespace mir {
namespace netcdf {



PackingCodec::PackingCodec(const Variable& variable):
    scale_factor_(0),
    add_offset_(0) {

}

PackingCodec::~PackingCodec() {

}

void PackingCodec::print(std::ostream &out) const {
    out << "PackingCodec[scale_factor=" << scale_factor_ << ", add_offset=" << add_offset_ << "]";
}


void PackingCodec::decode(std::vector<double> &v) const {
    for (size_t i = 0; i < v.size(); i++) {
        v[i] = v[i] * scale_factor_ + add_offset_;
    }
}


static CodecBuilder<PackingCodec> builder("packing");

}
}
