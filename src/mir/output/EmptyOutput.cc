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


#include <ostream>

#include "mir/output/EmptyOutput.h"


namespace mir {
namespace output {


EmptyOutput::EmptyOutput() {
}


EmptyOutput::~EmptyOutput() {
}


size_t EmptyOutput::copy(const param::MIRParametrisation&, context::Context&) {
    return 0;
}


size_t EmptyOutput::save(const param::MIRParametrisation&, context::Context&) {
    return 0;
}


bool EmptyOutput::sameAs(const MIROutput& other) const {
    const EmptyOutput* o = dynamic_cast<const EmptyOutput*>(&other);
    return o;
}


bool EmptyOutput::sameParametrisation(const param::MIRParametrisation&, const param::MIRParametrisation&) const {
    return false;
}


bool EmptyOutput::printParametrisation(std::ostream&, const param::MIRParametrisation&) const {
    return false;
}


void EmptyOutput::print(std::ostream& out) const {
    out << "EmptyOutput[]";
}


}  // namespace output
}  // namespace mir

