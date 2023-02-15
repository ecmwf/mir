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


#include "mir/key/packing/ArchivedValue.h"

#include "mir/util/Exceptions.h"


namespace mir::key::packing {


static const PackingBuilder<ArchivedValue> __packing("archived-value", "av", true, true);


ArchivedValue::ArchivedValue(const std::string& name, const param::MIRParametrisation& param) : Packing(name, param) {
    ASSERT(!definePacking_);
}


void ArchivedValue::fill(const repres::Representation* /*unused*/, grib_info& info) const {
    Packing::fill(info, 0 /* dummy, protected by ASSERT */);
}


void ArchivedValue::set(const repres::Representation* /*unused*/, grib_handle* handle) const {
    Packing::set(handle, "" /* dummy, protected by ASSERT */);
}


}  // namespace mir::key::packing
