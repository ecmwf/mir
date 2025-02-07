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


#include "mir/key/grid/SourcePattern.h"

#include <ostream>

#include "mir/key/intgrid/Source.h"
#include "mir/util/Exceptions.h"


namespace mir::key::grid {


SourcePattern::SourcePattern(const std::string& pattern) : GridPattern(pattern) {}


void SourcePattern::print(std::ostream& out) const {
    out << "SourcePattern[pattern=" << pattern_ << "]";
}


const Grid* SourcePattern::make(const std::string& name) const {
    NOTIMP;
}


std::string SourcePattern::canonical(const std::string& name, const param::MIRParametrisation& param) const {
    return param.fieldParametrisation().has("truncation") ? intgrid::Source(param, 0 /*unused*/).gridname() : "";
}


static const SourcePattern __pattern("^[sS][oO][uU][rR][cC][eE]$");


}  // namespace mir::key::grid
