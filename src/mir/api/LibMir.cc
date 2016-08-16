/*
 * (C) Copyright 1996-2016 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Baudouin Raoult
/// @author Tiago Quintino
/// @date   August 2016

#include "mir/api/LibMir.h"

#include "eckit/config/Resource.h"

using namespace eckit;

namespace mir {

//----------------------------------------------------------------------------------------------------------------------

LibMir::LibMir() : Library("mir") {}

eckit::PathName LibMir::cacheDir()
{
    return Resource<PathName>("mirCacheDir;$MIR_CACHE_DIR", "/tmp/cache");
}

const void* LibMir::addr() const { return this; }

static LibMir libmir;

//----------------------------------------------------------------------------------------------------------------------

} // namespace mir

