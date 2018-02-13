/*
 * (C) Copyright 1996-2015 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include "mir/util/MeshGeneratorParameters.h"

#include <limits>
#include "eckit/log/ResourceUsage.h"
#include "eckit/thread/Mutex.h"
#include "eckit/utils/MD5.h"
#include "atlas/mesh/Mesh.h"
#include "mir/caching/InMemoryCache.h"
#include "mir/config/LibMir.h"
#include "mir/param/MIRParametrisation.h"


namespace mir {
namespace util {


MeshGeneratorParameters::MeshGeneratorParameters() :
    meshGenerator_(""),  // defined by the representation
    patchNorthPole_(false),
    patchSouthPole_(false),
    meshXYZField_(true),
    file_("") {

    set("three_dimensional", true);
    set("patch_north_pole", patchNorthPole_);
    set("patch_south_pole", patchSouthPole_);
    set("triangulate",       false);
    set("angle",             0.);
}


void MeshGeneratorParameters::hash(eckit::MD5& md5) const {

    for (const char* p : {"three_dimensional", "patch_north_pole", "patch_south_pole", "triangulate"}) {
        bool value = false;
        ASSERT(get(p, value));
        md5 << value;
    }

    double angle = 0.;
    ASSERT(get("angle", angle));

    md5 << angle
        << meshGenerator_
        << meshXYZField_;
}


void MeshGeneratorParameters::print(std::ostream& s) const {
    s << "MeshGeneratorParameters["
      <<  "meshGenerator="     << meshGenerator_
      << ",patchNorthPole="    << patchNorthPole_
      << ",patchSouthPole="    << patchSouthPole_
      << ",meshXYZField="      << meshXYZField_
      << ",three_dimensional=" << getBool("three_dimensional")
      << ",triangulate="       << getBool("triangulate")
      << ",angle="             << getDouble("angle")
      << "]";
}


}  // namespace util
}  // namespace mir
