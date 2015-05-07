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


#include "eckit/thread/AutoLock.h"
#include "eckit/thread/Once.h"
#include "eckit/thread/Mutex.h"
#include "eckit/exception/Exceptions.h"

#include "mir/lsm/LandSeaMask.h"
#include "mir/param/MIRParametrisation.h"


namespace mir {
namespace lsm {

LandSeaMask::LandSeaMask(const std::string &name):
    name_(name) {
}


LandSeaMask::~LandSeaMask() {
}

//-----------------------------------------------------------------------------


}  // namespace lsm
}  // namespace mir

