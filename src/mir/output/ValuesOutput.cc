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



#include "mir/output/ValuesOutput.h"
#include "eckit/exception/Exceptions.h"
#include "mir/data/MIRField.h"
#include "mir/repres/Iterator.h"
#include "eckit/memory/ScopedPtr.h"
#include "mir/repres/Representation.h"
#include "mir/param/RuntimeParametrisation.h"
#include "eckit/io/HandleBuf.h"
#include "mir/action/context/Context.h"


namespace mir {
namespace output {

// See https://software.ecmwf.int/wiki/display/METV/Geopoints


ValuesOutput::ValuesOutput():
    hasMissing_(false),
    missingValue_(9999) {
}


ValuesOutput::~ValuesOutput() {
}


size_t ValuesOutput::copy(const param::MIRParametrisation &param, context::Context &ctx) {
    NOTIMP;
    return 0;
}

bool ValuesOutput::sameParametrisation(const param::MIRParametrisation &param1,
                                       const param::MIRParametrisation & param2) const {
    return true;
}

bool ValuesOutput::printParametrisation(std::ostream& out, const param::MIRParametrisation &param) const {
    return false;
}


bool ValuesOutput::sameAs(const MIROutput& other) const {
    const ValuesOutput* o = dynamic_cast<const ValuesOutput*>(&other);
    return o && this == o;
}


void ValuesOutput::print(std::ostream &out) const {
    out << "ValuesOutput[]";
}


size_t ValuesOutput::save(const param::MIRParametrisation &param, context::Context &ctx) {


    data::MIRField& field = ctx.field();

    ASSERT(field.dimensions() == 1);

    missingValue_ = field.missingValue();
    hasMissing_ = field.hasMissing();

    values_.resize(field.dimensions());

    for (size_t i = 0; i < field.dimensions(); ++i) {
        std::swap(values_[i], field.direct(i));
    }


    return 0;
}


bool ValuesOutput::hasMissing() const {
    return hasMissing_;
}

double ValuesOutput::missingValue() const {
    return missingValue_;
}

size_t ValuesOutput::dimensions() const {
    return values_.size();
}

const std::vector<double>& ValuesOutput::values(size_t which) const {
    ASSERT(which < values_.size());
    return values_[which];
}



}  // namespace output
}  // namespace mir

