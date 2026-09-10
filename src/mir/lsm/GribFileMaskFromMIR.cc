// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


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
