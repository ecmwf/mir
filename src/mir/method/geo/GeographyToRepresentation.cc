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


#include "mir/method/geo/GeographyToRepresentation.h"

#include <memory>
#include <set>

#include "mir/repres/Iterator.h"
#include "mir/repres/Representation.h"
#include "mir/repres/other/Geography.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Log.h"
#include "mir/util/Trace.h"


namespace mir {
namespace method {
namespace geo {


void GeographyToRepresentation::assemble(util::MIRStatistics& /*unused*/, WeightMatrix& W,
                                         const repres::Representation& in, const repres::Representation& out) const {
    // Note: 'in' is (must be) Geography
    auto& geo = dynamic_cast<const repres::other::Geography&>(in);

    std::set<Biplet> biplets;
    size_t j = 0;

    auto Nout = out.numberOfPoints();
    trace::ProgressTimer progress("assemble: assign from geography", Nout, {"point"});

    for (const std::unique_ptr<repres::Iterator> it(out.iterator()); it->next(); ++progress) {
        if (geo.inRegion(*(*it), j)) {
            biplets.emplace(it->index(), j);
        }
    }

    auto Nassigned = biplets.size();
    Log::debug() << "assemble: assigned: " << Nassigned << " of " << Log::Pretty(Nout, {"output point"}) << std::endl;

    // TODO: triplets, really? why not writing to the matrix directly?
    ASSERT_NONEMPTY_INTERPOLATION("GeographyToRepresentation", !biplets.empty());
    W.setFromTriplets({biplets.begin(), biplets.end()});
}


const char* GeographyToRepresentation::name() const {
    return "geography-point";
}


}  // namespace geo
}  // namespace method
}  // namespace mir
