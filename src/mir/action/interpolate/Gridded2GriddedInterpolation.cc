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
/// @author Pedro Maciel
/// @date Apr 2015


#include "mir/action/interpolate/Gridded2GriddedInterpolation.h"

#include "eckit/memory/ScopedPtr.h"

#include "atlas/grid/Grid.h"

#include "mir/data/MIRField.h"
#include "mir/method/Method.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/repres/Representation.h"
#include "mir/log/MIR.h"


namespace mir {
namespace action {


Gridded2GriddedInterpolation::Gridded2GriddedInterpolation(const param::MIRParametrisation &parametrisation):
    Action(parametrisation) {
}


Gridded2GriddedInterpolation::~Gridded2GriddedInterpolation() {
}


void Gridded2GriddedInterpolation::execute(data::MIRField & field, util::MIRStatistics& statistics) const {

    std::string interpolation;
    ASSERT(parametrisation_.get("interpolation", interpolation));

    eckit::ScopedPtr< method::Method > method(method::MethodFactory::build(interpolation, parametrisation_));
    eckit::Log::trace<MIR>() << "Method is " << *method << std::endl;

    repres::RepresentationHandle in(field.representation());
    repres::RepresentationHandle out(outputRepresentation());

    eckit::ScopedPtr<atlas::grid::Grid> gin(in->atlasGrid()); // We do it here as ATLAS does not respect constness
    eckit::ScopedPtr<atlas::grid::Grid> gout(out->atlasGrid());

    method->execute(field, *gin, *gout);

    field.representation(out);

    // Make sure we crop to the input domain if not global
    in->cropToDomain(parametrisation_, field);
}


}  // namespace action
}  // namespace mir

