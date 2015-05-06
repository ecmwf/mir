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


#include "eckit/exception/Exceptions.h"

#include "mir/lsm/GribFileLSM.h"


namespace mir {
namespace lsm {

GribFileLSM::GribFileLSM(const param::MIRParametrisation &parametrisation):
    LandSeaMask(parametrisation) {
}


GribFileLSM::~GribFileLSM() {
}

void GribFileLSM::print(std::ostream& out) const {
    out << "GribFileLSM[]";
}

data::MIRField* GribFileLSM::field(const atlas::Grid &) const {
    NOTIMP;
}

std::string GribFileLSM::unique_id() const {
    NOTIMP;
}

bool GribFileLSM::cacheable() const {
    NOTIMP;
}


//-----------------------------------------------------------------------------


}  // namespace logic
}  // namespace mir

