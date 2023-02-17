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


#include "mir/lsm/GribFileMaskFromMIR.h"


namespace mir::lsm {

GribFileMaskFromMIR::GribFileMaskFromMIR(const std::string& name, const eckit::PathName& path,
                                         const param::MIRParametrisation& parametrisation,
                                         const repres::Representation& representation, const std::string& which) :
    GribFileMask(path, parametrisation, representation, which), name_(name) {}

bool GribFileMaskFromMIR::cacheable() const {
    return true;
}

std::string GribFileMaskFromMIR::cacheName() const {
    return name_;
}


}  // namespace mir::lsm
