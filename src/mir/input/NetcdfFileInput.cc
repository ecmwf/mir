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

#include <limits>

#include "mir/data/MIRField.h"
#include "mir/netcdf/Field.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Log.h"
#include "mir/util/ValueMap.h"


namespace mir::input {


NetcdfFileInput::NetcdfFileInput(const eckit::PathName& path) :
    path_(path),
    cache_(*this),
    dataset_(path, *this),
    fields_(dataset_.fields()),
    current_(-1),
    checkDuplicatePoints_(false) {

    for (auto* field : fields_) {
        Log::info() << "NC " << *field << std::endl;
    }
}


NetcdfFileInput::~NetcdfFileInput() {
    for (auto* field : fields_) {
        delete field;
    }
}


const netcdf::Field& NetcdfFileInput::currentField() const {
    ASSERT(0 <= current_ && current_ < static_cast<int>(fields_.size()));
    return *fields_[static_cast<size_t>(current_)];
}


void NetcdfFileInput::setAuxiliaryInformation(const util::ValueMap& map) {
    auto it = map.find("checkDuplicatePoints");
    if (it != map.end()) {
        checkDuplicatePoints_ = it->second;
    }
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
    cache_.set("checkDuplicatePoints", checkDuplicatePoints_);

    FieldParametrisation::reset();

    current_++;
    return static_cast<size_t>(current_) < fields_.size();
}


data::MIRField NetcdfFileInput::field() const {
    const auto& ncField = currentField();

    auto hasMissing         = ncField.hasMissing();
    auto mv                 = ncField.missingValue();
    auto modifyMissingValue = hasMissing && mv != mv;

    if (modifyMissingValue) {
        mv = std::numeric_limits<double>::lowest();
        Log::warning() << "Modifying missing value from NaN to " << mv << std::endl;
    }

    data::MIRField field(cache_, hasMissing, mv);

    for (size_t i = 0; i < ncField.count2DValues(); ++i) {
        MIRValuesVector values;
        ncField.get2DValues(values, i);
        ncField.setMetadata(field, i);

        if (modifyMissingValue) {
            for (auto& v : values) {
                if (v != v) {
                    v = mv;
                }
            }
        }

        field.update(values, i);
    }

    return field;
}


bool NetcdfFileInput::has(const std::string& name) const {
    return currentField().has(name) || FieldParametrisation::has(name);
}


bool NetcdfFileInput::get(const std::string& name, std::string& value) const {
    return currentField().get(name, value) || FieldParametrisation::get(name, value);
}


bool NetcdfFileInput::get(const std::string& name, bool& value) const {
    return FieldParametrisation::get(name, value);
}


bool NetcdfFileInput::get(const std::string& name, int& value) const {
    return FieldParametrisation::get(name, value);
}


bool NetcdfFileInput::get(const std::string& name, long& value) const {
    return currentField().get(name, value) || FieldParametrisation::get(name, value);
}


bool NetcdfFileInput::get(const std::string& name, float& value) const {
    return FieldParametrisation::get(name, value);
}


bool NetcdfFileInput::get(const std::string& name, double& value) const {
    return currentField().get(name, value) || FieldParametrisation::get(name, value);
}


bool NetcdfFileInput::get(const std::string& name, std::vector<int>& value) const {
    return FieldParametrisation::get(name, value);
}


bool NetcdfFileInput::get(const std::string& name, std::vector<long>& value) const {
    return FieldParametrisation::get(name, value);
}


bool NetcdfFileInput::get(const std::string& name, std::vector<float>& value) const {
    return FieldParametrisation::get(name, value);
}


bool NetcdfFileInput::get(const std::string& name, std::vector<double>& value) const {
    return currentField().get(name, value) || FieldParametrisation::get(name, value);
}


bool NetcdfFileInput::get(const std::string& name, std::vector<std::string>& value) const {
    return FieldParametrisation::get(name, value);
}


bool NetcdfFileInput::sameAs(const MIRInput& other) const {
    const auto* o = dynamic_cast<const NetcdfFileInput*>(&other);
    return (o != nullptr) && (path_ == o->path_);
}


size_t NetcdfFileInput::dimensions() const {
    return currentField().count2DValues();
}


static const MIRInputBuilder<NetcdfFileInput> netcdf4(0x89484446);   // ".HDF"
static const MIRInputBuilder<NetcdfFileInput> netcdf31(0x43444601);  // "CDF."
static const MIRInputBuilder<NetcdfFileInput> netcdf32(0x43444602);  // "CDF."


}  // namespace mir::input
