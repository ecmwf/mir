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


#include <istream>

#include "eckit/io/Length.h"

#include "mir/output/DummyOutput.h"


namespace mir {
namespace output {


DummyOutput::DummyOutput() {
}


DummyOutput::~DummyOutput() {
}


bool DummyOutput::sameAs(const MIROutput& other) const {
    const DummyOutput* o = dynamic_cast<const DummyOutput*>(&other);
    return o;
}


void DummyOutput::copy(const param::MIRParametrisation &param, input::MIRInput &input) {
}


void DummyOutput::save(const param::MIRParametrisation &param, input::MIRInput &input, data::MIRField &field) {
}


void DummyOutput::print(std::ostream &out) const {
    out << "DummyOutput[...]";
}

eckit::Length DummyOutput::total() const {
    return 0;
}


}  // namespace output
}  // namespace mir

