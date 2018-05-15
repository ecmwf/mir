/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include "mir/util/MeshGeneratorParameters.h"

#include "eckit/utils/MD5.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/repres/Representation.h"


namespace mir {
namespace util {


MeshGeneratorParameters::MeshGeneratorParameters() :
    meshGenerator_(""),
    meshParallelEdgesConnectivity_(true),
    meshXYZField_(true),
    meshCellCentres_(true),
    file_("") {
    set("three_dimensional",  true);
    set("patch_pole",         true);
    set("include_pole",       false);
    set("triangulate",        false);
    set("angle",             0.);
}


MeshGeneratorParameters::MeshGeneratorParameters(const std::string& label, const param::MIRParametrisation& param) :
    MeshGeneratorParameters() {
    ASSERT(!label.empty());
    const param::MIRParametrisation& user = param.userParametrisation();

    user.get(label + "-mesh-add-parallel-edges-connectivity", meshParallelEdgesConnectivity_);
    user.get(label + "-mesh-add-field-xyz", meshXYZField_);
    user.get(label + "-mesh-add-field-cell-centres", meshCellCentres_);
    user.get(label + "-mesh-generator", meshGenerator_);
    user.get(label + "-mesh-file", file_);
}


void MeshGeneratorParameters::setOptions(const repres::Representation& representation) {
    representation.fill(*this);
}


bool MeshGeneratorParameters::sameAs(const MeshGeneratorParameters& other) const {
    eckit::MD5 a, b;
    hash(a);
    other.hash(b);

    return a.digest() == b.digest();
}


void MeshGeneratorParameters::hash(eckit::Hash& hash) const {

    for (const char* p : {"three_dimensional", "patch_pole", "include_pole", "triangulate"}) {
        bool value = false;
        ASSERT(get(p, value));
        hash << value;
    }

    double angle = 0.;
    ASSERT(get("angle", angle));

    hash << angle
        << meshGenerator_
        << meshParallelEdgesConnectivity_
        << meshXYZField_
        << meshCellCentres_;
}


void MeshGeneratorParameters::print(std::ostream& s) const {
    s << "MeshGeneratorParameters["
      <<  "meshGenerator="                 << meshGenerator_
      << ",meshParallelEdgesConnectivity=" << meshParallelEdgesConnectivity_
      << ",meshXYZField="                  << meshXYZField_
      << ",meshCellCentres="               << meshCellCentres_
      << ",three_dimensional=" << getBool("three_dimensional")
      << ",patch_pole="        << getBool("patch_pole")
      << ",include_pole="      << getBool("include_pole")
      << ",triangulate="       << getBool("triangulate")
      << ",angle="             << getDouble("angle")
      << "]";
}


}  // namespace util
}  // namespace mir
