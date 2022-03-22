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


#include "mir/method/geo/RepresentationToGeography.h"

#include <memory>
#include <set>

#include "mir/method/solver/Statistics.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/repres/Iterator.h"
#include "mir/repres/Representation.h"
#include "mir/repres/other/Geography.h"
#include "mir/stats/Field.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Log.h"
#include "mir/util/Trace.h"


namespace mir {
namespace method {
namespace geo {


RepresentationToGeography::RepresentationToGeography(const param::MIRParametrisation& param) : GeographyMethod(param) {
    std::string stats = "maximum";
    param.get("interpolation-statistics", stats);

    setSolver(new solver::Statistics(param, stats::FieldFactory::build(stats, param)));
}


void RepresentationToGeography::assemble(util::MIRStatistics& /*unused*/, WeightMatrix& W,
                                         const repres::Representation& in, const repres::Representation& out) const {
    // Note: 'out' is (must be) Geography
    auto& geo = dynamic_cast<const repres::other::Geography&>(out);

    std::set<Biplet> biplets;
    size_t i = 0;

    auto Nin = in.numberOfPoints();
    trace::ProgressTimer progress("assemble: assign to geography", Nin, {"point"});

    for (const std::unique_ptr<repres::Iterator> it(in.iterator()); it->next(); ++progress) {
        if (geo.inRegion(*(*it), i)) {
            biplets.emplace(i, it->index());
        }
    }

    auto Nassigned = biplets.size();
    Log::debug() << "assemble: assigned: " << Nassigned << " of " << Log::Pretty(Nin, {"input point"}) << std::endl;

    // TODO: triplets, really? why not writing to the matrix directly?
    ASSERT_NONEMPTY_INTERPOLATION("RepresentationToGeography", !biplets.empty());
    W.setFromTriplets({biplets.begin(), biplets.end()});
}


const char* RepresentationToGeography::name() const {
    return "geography-point";
}


}  // namespace geo
}  // namespace method
}  // namespace mir
