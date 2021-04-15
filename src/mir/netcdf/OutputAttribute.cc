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


#include "mir/netcdf/OutputAttribute.h"

#include <ostream>

#include "mir/netcdf/Endowed.h"
#include "mir/netcdf/Exceptions.h"
#include "mir/netcdf/Value.h"
#include "mir/util/Log.h"


namespace mir {
namespace netcdf {
/*
See http://www.unidata.ucar.edu/software/netcdf/docs/netcdf/Attribute-Conventions.html
*/


static const char* dont_drop[] = {"_FillValue", "missing_value", nullptr};


OutputAttribute::OutputAttribute(Endowed& owner, const std::string& name, Value* value) :
    Attribute(owner, name, value), valid_(true) {}


OutputAttribute::~OutputAttribute() = default;


void OutputAttribute::create(int nc) const {
    if (valid_) {
        value_->createAttribute(nc, owner_.varid(), name_, owner_.path());
    }
}


void OutputAttribute::clone(Endowed& owner) const {
    owner.add(new OutputAttribute(owner, name_, value_->clone()));
}


void OutputAttribute::merge(const Attribute& other) {
    if (!value_->sameAs(other.value())) {
        if (valid_) {
            Log::warning() << "WARNING: dropping attribute " << fullName() << std::endl;
            valid_ = false;

            for (size_t i = 0; dont_drop[i] != nullptr; ++i) {
                if (name_ == dont_drop[i]) {
                    throw exception::MergeError("Attempt to drop attribute " + name_);
                }
            }
        }
    }
}


void OutputAttribute::invalidate() {
    valid_ = false;
}


void OutputAttribute::print(std::ostream& out) const {
    out << "OutputAttribute[name=" << name_ << "]";
}

}  // namespace netcdf
}  // namespace mir
