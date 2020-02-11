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


#include "mir/input/NetcdfFileInput.h"

#include "eckit/exception/Exceptions.h"
#include "mir/data/MIRField.h"
#include "mir/netcdf/Field.h"
#include "mir/util/Grib.h"


namespace mir {
namespace input {


NetcdfFileInput::NetcdfFileInput(const eckit::PathName& path) :
    path_(path),
    cache_(*this),
    dataset_(path, *this),
    fields_(dataset_.fields()),
    current_(-1) {

    for (auto& field : fields_) {
        std::cout << "NC " << *field << std::endl;
    }
}


NetcdfFileInput::~NetcdfFileInput() {
    for (auto& field : fields_) {
        delete field;
    }
}


grib_handle* NetcdfFileInput::gribHandle(size_t which) const {
    // ASSERT(which == 0);
    static grib_handle* handle = nullptr;
    if (handle == nullptr) {
        handle = grib_handle_new_from_samples(nullptr, "GRIB1");
        grib_set_long(handle, "paramId", 255);
        ASSERT(handle);
    }
    return handle;
}


void NetcdfFileInput::print(std::ostream& out) const {
    out << "NetcdfFileInput[path=" << path_ << "]";
}


const param::MIRParametrisation& NetcdfFileInput::parametrisation(size_t which) const {
    ASSERT(which == 0);
    return cache_;
}

bool NetcdfFileInput::next() {
    cache_.reset();
    FieldParametrisation::reset();

    current_++;

    return current_ < int(fields_.size());
}


data::MIRField NetcdfFileInput::field() const {
    ASSERT(current_ >= 0 && (current_ < int(fields_.size())));

    auto& ncField = *fields_[current_];

    data::MIRField field(*this, ncField.hasMissing(), ncField.missingValue());

    size_t n = ncField.count2DValues();
    for (size_t i = 0; i < n; ++i) {
        MIRValuesVector values;
        ncField.get2DValues(values, i);
        field.update(values, i);

        ncField.setMetadata(field, i);
    }

    return field;
}

bool NetcdfFileInput::get(const std::string& name, long& value) const {
    ASSERT(current_ >= 0 && current_ < fields_.size());
    if (fields_[current_]->get(name, value)) {
        return true;
    }
    return FieldParametrisation::get(name, value);
}

bool NetcdfFileInput::has(const std::string& name) const {
    ASSERT(current_ >= 0 && current_ < fields_.size());
    if (fields_[current_]->has(name)) {
        return true;
    }
    return FieldParametrisation::has(name);
}

bool NetcdfFileInput::get(const std::string& name, std::string& value) const {
    ASSERT(current_ >= 0 && current_ < fields_.size());
    if (fields_[current_]->get(name, value)) {
        return true;
    }
    return FieldParametrisation::get(name, value);
}

bool NetcdfFileInput::get(const std::string& name, bool& value) const {

    // NOTE: this disables checking for duplicate points for any NetCDF file!!
    if (name == "check-duplicate-points") {
        value = false;
        return true;
    }

    return false;
}

bool NetcdfFileInput::get(const std::string& name, double& value) const {
    ASSERT(current_ >= 0 && current_ < fields_.size());
    if (fields_[current_]->get(name, value)) {
        return true;
    }
    return FieldParametrisation::get(name, value);
}


bool NetcdfFileInput::get(const std::string& name, std::vector<double>& value) const {
    ASSERT(current_ >= 0 && current_ < fields_.size());
    if (fields_[current_]->get(name, value)) {
        return true;
    }
    return FieldParametrisation::get(name, value);
}

bool NetcdfFileInput::sameAs(const MIRInput& other) const {
    auto o = dynamic_cast<const NetcdfFileInput*>(&other);
    return (o != nullptr) && (path_ == o->path_);
}

size_t NetcdfFileInput::dimensions() const {
    ASSERT(current_ >= 0 && current_ < fields_.size());
    return fields_[current_]->count2DValues();
}


static MIRInputBuilder<NetcdfFileInput> netcdf4(0x89484446);   // ".HDF"
static MIRInputBuilder<NetcdfFileInput> netcdf31(0x43444601);  // "CDF."
static MIRInputBuilder<NetcdfFileInput> netcdf32(0x43444602);  // "CDF."


}  // namespace input
}  // namespace mir
