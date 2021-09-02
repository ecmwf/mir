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


#include "mir/method/voronoi/VoronoiAverage.h"

#include <vector>

//#include "Qhull.h"
////#include "QhullError.h"
////#include "QhullFacet.h"
//#include "QhullFacetList.h"
////#include "QhullQh.h"
////#include "QhullSet.h"
////#include "QhullVertex.h"
////#include "QhullVertexSet.h"
//#include "RboxPoints.h"

#include "mir/param/MIRParametrisation.h"
#include "mir/stats/Field.h"


namespace mir {
namespace method {
namespace voronoi {


VoronoiAverage::VoronoiAverage(const param::MIRParametrisation& param) : VoronoiMethod(param) {}


void VoronoiAverage::assemble(util::MIRStatistics&, WeightMatrix& W, const repres::Representation& in,
                              const repres::Representation& out) const {
#if 0
    using namespace orgQhull;

    RboxPoints rbox;
    rbox.appendPoints("100");

    const char* inputComment2 = "";
    const char* qhullCommand2 = "";
    int pointDimension = 3;
    int pointCount = 3;

    std::vector<int> pointCoordinates = {0, 0, 0, 0, 0, 0, 0, 0, 0, };

    Qhull qhull(inputComment2, pointDimension, pointCount, pointCoordinates.data(), qhullCommand2);

    qhull.runQhull(rbox, "");
    std::cout << qhull.facetList();
#endif
}


static MethodBuilder<VoronoiAverage> __builder("voronoi-average");


}  // namespace voronoi
}  // namespace method
}  // namespace mir
