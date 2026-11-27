// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/method/voronoi/VoronoiMethod.h"

#include <algorithm>
#include <ostream>
#include <set>
#include <sstream>
#include <utility>
#include <variant>

#include "eckit/geo/Grid.h"
#include "eckit/geo/Search.h"
#include "eckit/log/JSON.h"
#include "eckit/utils/MD5.h"

#include "mir/repres/Representation.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Log.h"
#include "mir/util/LonLatToXYZ.h"
#include "mir/util/Search.h"
#include "mir/util/Trace.h"


namespace mir::method::voronoi {


namespace {


struct Biplet : std::pair<size_t, size_t> {
    using pair::pair;
    operator WeightMatrix::Triplet() const { return {first, second, 1. /*non-zero*/}; }
    bool operator<(const Biplet& other) const {
        return first < other.first || (first == other.first && second < other.second);
    }
};


}  // namespace


VoronoiMethod::VoronoiMethod(const param::MIRParametrisation& param) : MethodWeighted(param), pick_(1) {}


bool VoronoiMethod::sameAs(const Method& other) const {
    const auto* o = dynamic_cast<const VoronoiMethod*>(&other);
    return (o != nullptr) && type() == o->type() && MethodWeighted::sameAs(*o);
}


void VoronoiMethod::assemble(util::MIRStatistics& /*unused*/, WeightMatrix& W, const repres::Representation& in,
                             const repres::Representation& out) const {
    auto& log = Log::debug();
    log << "VoronoiMethod::assemble (input: " << in << ", output: " << out << ")" << std::endl;


    std::unique_ptr<const eckit::geo::Search> tree;
    {
        trace::ResourceUsage usage("assemble: create output k-d tree");
        tree = util::make_search(parametrisation_, out);
    }


    // points on the figure of the points searched
    const util::LonLatToXYZ to_out(out.canonicalGrid().figure(), poleDisplacement(in));
    const util::LonLatToXYZ to_in(in.canonicalGrid().figure(), poleDisplacement(in));

    auto Nin  = in.numberOfPoints();
    auto Nout = out.numberOfPoints();

    std::vector<bool> assigned;
    assigned.assign(Nout, false);

    std::set<Biplet> biplets;


    {
        trace::ProgressTimer progress("assemble: input-based assign", Nin, {"point"});

        eckit::geo::Search::Neighbours closest;
        const auto& points = in.canonicalGrid();
        for (auto it = points.cbegin(); it; ++it) {
            if (++progress) {
                log << *tree << std::endl;
            }

            // lookup
            pick_.pick(*tree, to_out(std::get<PointLonLat>(*it)), closest);
            for (auto& c : closest) {
                auto i = c.index;
                biplets.emplace(i, it.index());
                assigned[i] = true;
            }
        }
    }


    auto Nassigned = size_t(std::count(assigned.cbegin(), assigned.cend(), true));
    if (Nassigned < Nout) {
        Log::debug() << "assemble: input-based assignment: " << Nassigned << " of "
                     << Log::Pretty(Nout, {"output point"}) << " (completing with output-based assignment)"
                     << std::endl;

        {
            trace::ResourceUsage usage("assemble: create input k-d tree");
            tree = util::make_search(parametrisation_, in);
        }

        {
            trace::ProgressTimer progress("assemble: output-based assign", Nout - Nassigned, {"point"});

            eckit::geo::Search::Neighbours closest;
            const auto& points = out.canonicalGrid();
            for (auto it = points.cbegin(); it; ++it) {
                auto i = it.index();
                if (assigned[i]) {
                    continue;
                }

                if (++progress) {
                    log << *tree << std::endl;
                }

                // lookup
                pick_.pick(*tree, to_in(std::get<PointLonLat>(*it)), closest);
                for (auto& c : closest) {
                    auto j = c.index;
                    biplets.emplace(i, j);  // won't insert biplet if existing
                }
            }
        }
    }

    {
        trace::Timer time("assemble: fill sparse matrix");

        // TODO: triplets, really? why not writing to the matrix directly?
        ASSERT_NONEMPTY_INTERPOLATION("VoronoiMethod", !biplets.empty());
        W.setFromTriplets({biplets.begin(), biplets.end()});
    }
}


void VoronoiMethod::hash(eckit::MD5& md5) const {
    MethodWeighted::hash(md5);
    std::ostringstream str;
    print(str);
    md5.add(str.str());
}


void VoronoiMethod::print(std::ostream& out) const {
    out << "VoronoiMethod[type=" << type() << ",";
    MethodWeighted::print(out);
    out << "]";
}


WeightMatrix::Check VoronoiMethod::validateMatrixWeights() const {
    return {true, true, false};
}


int VoronoiMethod::version() const {
    return 0;
}


}  // namespace mir::method::voronoi
