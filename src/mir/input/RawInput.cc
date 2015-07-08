/*
 * (C) Copyright 1996-2015 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Baudouin Raoult
/// @author Pedro Maciel
/// @date Apr 2015


#include <cmath>
#include <iostream>

#include "mir/data/MIRField.h"

#include "mir/input/RawInput.h"
#include "eckit/exception/Exceptions.h"


namespace mir {
namespace input {


RawInput::RawInput(const RawMetadata &metadata, const double *values, size_t count):
    metadata_(metadata),
    values_(values),
    count_(count) {
}

RawInput::~RawInput() {
}

bool RawInput::next() {
    NOTIMP;
}

const param::MIRParametrisation &RawInput::parametrisation() const {
    return *this;
}

data::MIRField *RawInput::field() const {
    data::MIRField *field = new data::MIRField(*this, false, 999.);

    std::vector< double > values(count_);
    ::memcpy(&values[0], values_, sizeof(double) * count_);
    field->values(values, 0);

    return field;
}


void RawInput::print(std::ostream &out) const {
    out << "RawInput[count=" << count_ << "]";
}

size_t RawInput::copy(double *values, size_t size) const {
    ASSERT(count_ <= size);
    ::memcpy(values, values_, sizeof(double) * count_);
    return count_;
}

bool RawInput::has(const std::string &name) const {
    eckit::Log::info() << ">>>>>>>>>>>>> RawInput::has (" << name << ")" << std::endl;

    if (name == "gridded") {
        return metadata_.gridded();
    }

    if (name == "spectral") {
        return metadata_.spectral();
    }

    return false;
}

bool RawInput::get(const std::string &name, std::string &value) const {
    eckit::Log::info() << ">>>>>>>>>>>>> RawInput::get string (" << name << ")" << std::endl;

    if (name == "gridType") {
        value = metadata_.gridType();
        return true;
    }

    return false;
}

bool RawInput::get(const std::string &name, bool &value) const {
    eckit::Log::info() << ">>>>>>>>>>>>> RawInput::get bool (" << name << ")" << std::endl;
    return false;
}

bool RawInput::get(const std::string &name, long &value) const {
    eckit::Log::info() << ">>>>>>>>>>>>> RawInput::get long (" << name << ")" << std::endl;

    if (name == "N") {
        value = metadata_.N();
        return true;
    }

    if (name == "Nj") {
        value = metadata_.nj();
        return true;
    }

    if (name == "truncation") {
        value = metadata_.truncation();
        return true;
    }

    return false;
}

bool RawInput::get(const std::string &name, double &value) const {
    eckit::Log::info() << ">>>>>>>>>>>>> RawInput::get double (" << name << ")" << std::endl;

    if (name == "north") {
        value = metadata_.bbox().north();
        return true;
    }

    if (name == "south") {
        value = metadata_.bbox().south();
        return true;
    }

    if (name == "west") {
        value = metadata_.bbox().west();
        return true;
    }

    if (name == "east") {
        value = metadata_.bbox().east();
        return true;
    }

    return false;
}

bool RawInput::get(const std::string &name, std::vector<long> &value) const {
    eckit::Log::info() << ">>>>>>>>>>>>> RawInput::get vector<long> (" << name << ")" << std::endl;

    if (name == "pl") {
        value = metadata_.pl();
        return true;
    }

    return false;
}

bool RawInput::get(const std::string &name, std::vector<double> &value) const {

    if (name == "area") {
        value.resize(4);
        value[0] = metadata_.bbox().north(); // North
        value[1] = metadata_.bbox().west(); // West
        value[2] = metadata_.bbox().south(); // South
        value[3] = metadata_.bbox().east(); // East
        return true;
    }

    if (name == "grid") {
        return false;
    }

    eckit::Log::info() << ">>>>>>>>>>>>> RawInput::get vector<double> (" << name << ")" << std::endl;
    return false;
}

}  // namespace input
}  // namespace mir

