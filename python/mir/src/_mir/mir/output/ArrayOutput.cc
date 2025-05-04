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


#include "mir/output/ArrayOutput.h"

#include <ostream>
#include <sstream>

#include "eckit/geo/Exceptions.h"
#include "eckit/geo/Grid.h"
#include "eckit/log/JSON.h"

#include "mir/action/context/Context.h"
#include "mir/api/MIRJob.h"
#include "mir/data/MIRField.h"
#include "mir/param/GridSpecParametrisation.h"
#include "mir/repres/Representation.h"


namespace mir::output {


size_t ArrayOutput::save(const param::MIRParametrisation&, context::Context& ctx) {
    const auto& field = ctx.field();

    // save values
    ASSERT(field.dimensions() == 1);
    field.validate();
    values_ = field.values(0);

    // save gridspec (a hack)
    api::MIRJob job;
    repres::RepresentationHandle(field.representation())->fillJob(job);

    std::ostringstream spec;
    eckit::JSON j(spec);
    j << job;

    std::unique_ptr<const eckit::geo::Grid> grid(eckit::geo::GridFactory::make_from_string(spec.str()));
    ASSERT(grid);

    shape_    = grid->shape();
    gridspec_ = grid->spec_str();

    return 1;
}


void ArrayOutput::print(std::ostream& out) const {
    out << "ArrayOutput[#values=" << values_.size() << ",shape=[";
    const auto* sep = "";
    for (auto s : shape_) {
        out << sep << s;
        sep = ",";
    }
    out << "],gridspec=[" << gridspec_ << "]]";
}


}  // namespace mir::output
