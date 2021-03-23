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


#include "mir/method/voronoi/VoronoiMethod.h"

#include <memory>
#include <ostream>
#include <set>
#include <sstream>
#include <utility>

#include "eckit/utils/MD5.h"

#include "mir/repres/Iterator.h"
#include "mir/repres/Representation.h"
#include "mir/search/PointSearch.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Log.h"
#include "mir/util/Trace.h"


namespace mir {
namespace method {
namespace voronoi {


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
    auto o = dynamic_cast<const VoronoiMethod*>(&other);
    return (o != nullptr) && name() == o->name() && MethodWeighted::sameAs(*o);
}


void VoronoiMethod::assemble(util::MIRStatistics&, WeightMatrix& W, const repres::Representation& in,
                             const repres::Representation& out) const {
    auto& log = Log::debug();
    log << "VoronoiMethod::assemble (input: " << in << ", output: " << out << ")" << std::endl;


    std::unique_ptr<search::PointSearch> tree;
    {
        trace::ResourceUsage usage("VoronoiMethod::assemble create k-d tree", log);
        tree.reset(new search::PointSearch(parametrisation_, in));
    }


    std::set<Biplet> biplets;

    {
        trace::ProgressTimer progress("VoronoiMethod::assemble create Voronoi", out.numberOfPoints(), {"point"}, log);

        std::vector<search::PointSearch::PointValueType> closest;
        size_t i = 0;
        for (std::unique_ptr<repres::Iterator> it(out.iterator()); it->next(); ++i) {
            if (++progress) {
                log << *tree << std::endl;
            }

            pick_.pick(*tree, it->point3D(), closest);
            for (auto& c : closest) {
                auto j = c.payload();
                biplets.emplace_hint(biplets.end(), i, j);
            }
        }
    }


    {
        trace::Timer time("VoronoiMethod::assemble fill sparse matrix", log);

        // TODO: triplets, really? why not writing to the matrix directly?
        W.setFromTriplets({biplets.begin(), biplets.end()});
    }
}


void VoronoiMethod::hash(eckit::MD5& md5) const {
    MethodWeighted::hash(md5);
    std::stringstream str;
    print(str);
    md5.add(str.str());
}


void VoronoiMethod::print(std::ostream& out) const {
    out << "VoronoiMethod[name=" << name() << ",";
    MethodWeighted::print(out);
    out << "]";
}


bool VoronoiMethod::validateMatrixWeights() const {
    return false;
}


const char* VoronoiMethod::name() const {
    return "voronoi";
}


}  // namespace voronoi
}  // namespace method
}  // namespace mir
