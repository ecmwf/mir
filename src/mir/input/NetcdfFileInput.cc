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

    for (auto field : fields_) {
        eckit::Log::info() << "NC " << *field << std::endl;
    }
}


NetcdfFileInput::~NetcdfFileInput() {
    for (auto field : fields_) {
        delete field;
    }
}


const netcdf::Field& NetcdfFileInput::currentField() const {
    ASSERT(0 <= current_ && current_ < int(fields_.size()));
    return *fields_[size_t(current_)];
}


grib_handle* NetcdfFileInput::gribHandle(size_t /*which*/) const {
    // ASSERT(which == 0);
    static grib_handle* handle = nullptr;
    if (handle == nullptr) {
        handle = codes_grib_handle_new_from_samples(nullptr, "GRIB1");
        codes_set_long(handle, "paramId", 255);
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
    return size_t(current_) < fields_.size();
}


data::MIRField NetcdfFileInput::field() const {

    auto& ncField = currentField();
    data::MIRField field(*this, ncField.hasMissing(), ncField.missingValue());

    for (size_t i = 0; i < ncField.count2DValues(); ++i) {
        MIRValuesVector values;
        ncField.get2DValues(values, i);
        ncField.setMetadata(field, i);
        field.update(values, i);
    }

    return field;
}


bool NetcdfFileInput::has(const std::string& name) const {
    return currentField().has(name) || FieldParametrisation::has(name);
}


bool NetcdfFileInput::get(const std::string& name, long& value) const {
    return currentField().get(name, value) || FieldParametrisation::get(name, value);
}


bool NetcdfFileInput::get(const std::string& name, std::string& value) const {
    return currentField().get(name, value) || FieldParametrisation::get(name, value);
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
    return currentField().get(name, value) || FieldParametrisation::get(name, value);
}


bool NetcdfFileInput::get(const std::string& name, std::vector<double>& value) const {
    return currentField().get(name, value) || FieldParametrisation::get(name, value);
}


bool NetcdfFileInput::sameAs(const MIRInput& other) const {
    auto o = dynamic_cast<const NetcdfFileInput*>(&other);
    return (o != nullptr) && (path_ == o->path_);
}


size_t NetcdfFileInput::dimensions() const {
    return currentField().count2DValues();
}


static MIRInputBuilder<NetcdfFileInput> netcdf4(0x89484446);   // ".HDF"
static MIRInputBuilder<NetcdfFileInput> netcdf31(0x43444601);  // "CDF."
static MIRInputBuilder<NetcdfFileInput> netcdf32(0x43444602);  // "CDF."


}  // namespace input
}  // namespace mir
