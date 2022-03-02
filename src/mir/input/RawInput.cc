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


#include "mir/input/RawInput.h"

#include <cstring>
#include <ostream>

#include "mir/data/MIRField.h"
#include "mir/input/RawMetadata.h"
#include "mir/util/BoundingBox.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Log.h"


namespace mir {
namespace input {


RawInput::RawInput(const RawMetadata& metadata, const double* values, size_t count) :
    metadata_(metadata), values_(values), count_(count) {}


bool RawInput::sameAs(const MIRInput& other) const {
    return this == &other;
}


bool RawInput::next() {
    NOTIMP;
}


const param::MIRParametrisation& RawInput::parametrisation(size_t which) const {
    ASSERT(which == 0);
    return *this;
}


data::MIRField RawInput::field() const {

    data::MIRField field(*this, metadata_.hasMissing(), metadata_.missingValue());

    MIRValuesVector values(count_);
    std::memcpy(&values[0], values_, sizeof(double) * count_);
    field.update(values, 0);

    // Log::debug() << "RawInput::field: " << field << std::endl;

    return field;
}


void RawInput::print(std::ostream& out) const {
    out << "RawInput[count=" << count_ << "]";
}


size_t RawInput::copy(double* values, size_t size) const {
    ASSERT(count_ <= size);
    std::memcpy(values, values_, sizeof(double) * count_);
    return count_;
}


bool RawInput::has(const std::string& name) const {
    Log::debug() << ">>>>>>>>>>>>> RawInput::has (" << name << ")" << std::endl;

    if (name == "gridded") {
        return metadata_.gridded();
    }

    if (name == "spectral") {
        return metadata_.spectral();
    }

    return false;
}


bool RawInput::get(const std::string& name, std::string& value) const {
    Log::debug() << ">>>>>>>>>>>>> RawInput::get string (" << name << ")" << std::endl;

    if (name == "gridType") {
        value = metadata_.gridType();
        return true;
    }

    return false;
}


bool RawInput::get(const std::string& name, bool& /*value*/) const {
    Log::debug() << ">>>>>>>>>>>>> RawInput::get bool (" << name << ")" << std::endl;
    return false;
}


bool RawInput::get(const std::string& name, int& value) const {
    long v;
    if (get(name, v)) {
        ASSERT(long(int(v)) == v);
        value = int(v);
        return true;
    }
    return false;
}


bool RawInput::get(const std::string& name, long& value) const {
    Log::debug() << ">>>>>>>>>>>>> RawInput::get long (" << name << ")" << std::endl;

    if (name == "N") {
        value = long(metadata_.N());
        return true;
    }

    if (name == "Nj") {
        value = long(metadata_.nj());
        return true;
    }

    if (name == "truncation") {
        value = long(metadata_.truncation());
        return true;
    }

    if (name == "paramId") {
        value = long(metadata_.paramId());
        return true;
    }

    return false;
}


bool RawInput::get(const std::string& name, float& value) const {
    double v;
    if (get(name, v)) {
        value = float(v);
        return true;
    }
    return false;
}


bool RawInput::get(const std::string& name, double& value) const {
    Log::debug() << ">>>>>>>>>>>>> RawInput::get double (" << name << ")" << std::endl;

    if (name == "north") {
        value = metadata_.bbox().north().value();
        return true;
    }

    if (name == "south") {
        value = metadata_.bbox().south().value();
        return true;
    }

    if (name == "west") {
        value = metadata_.bbox().west().value();
        return true;
    }

    if (name == "east") {
        value = metadata_.bbox().east().value();
        return true;
    }

    return false;
}


bool RawInput::get(const std::string& name, std::vector<int>& value) const {
    std::vector<long> v;
    if (get(name, v)) {
        value.clear();
        value.reserve(v.size());
        for (const long& l : v) {
            ASSERT(long(int(l)) == l);
            value.push_back(int(l));
        }
        return true;
    }
    return false;
}


bool RawInput::get(const std::string& name, std::vector<long>& value) const {
    Log::debug() << ">>>>>>>>>>>>> RawInput::get vector<long> (" << name << ")" << std::endl;

    if (name == "pl") {
        value = metadata_.pl();
        return true;
    }

    return false;
}


bool RawInput::get(const std::string& name, std::vector<float>& value) const {
    std::vector<double> v;
    if (get(name, v)) {
        value.clear();
        value.reserve(v.size());
        for (const double& l : v) {
            value.push_back(float(l));
        }
        return true;
    }
    return false;
}


bool RawInput::get(const std::string& name, std::vector<double>& value) const {

    if (name == "area") {
        value.resize(4);
        value[0] = metadata_.bbox().north().value();  // North
        value[1] = metadata_.bbox().west().value();   // West
        value[2] = metadata_.bbox().south().value();  // South
        value[3] = metadata_.bbox().east().value();   // East
        return true;
    }

    if (name == "grid") {
        return false;
    }

    Log::debug() << ">>>>>>>>>>>>> RawInput::get vector<double> (" << name << ")" << std::endl;
    return false;
}


bool RawInput::get(const std::string& /*name*/, std::vector<std::string>& /*value*/) const {
    NOTIMP;
}


}  // namespace input
}  // namespace mir
