/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include "mir/method/gridbox/GridBoxMethod.h"

#include <algorithm>
#include <map>
#include <sstream>
#include <vector>

#include "eckit/exception/Exceptions.h"
#include "eckit/log/Log.h"
#include "eckit/log/ProgressTimer.h"
#include "eckit/types/FloatCompare.h"
#include "eckit/utils/MD5.h"

#include "mir/api/Atlas.h"
#include "mir/config/LibMir.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/repres/Representation.h"
#include "mir/util/Angles.h"
#include "mir/util/Domain.h"
#include "mir/util/GridBox.h"
#include "mir/util/Pretty.h"


namespace mir {
namespace method {
namespace gridbox {


// std::vector<GridBox> boxes(LatitudeRange lat, std::vector<double> lonEdges) {
//    ASSERT(lonEdges.size() > 1);

//    std::vector<GridBox> boxes;
//    boxes.reserve(lonEdges.size() - 1);
//    for (size_t i = 0; i < lonEdges.size() - 1; ++i) {
//        LongitudeRange lon{lonEdges[i], lonEdges[i + 1]};
//        boxes.emplace_back(lat, lon);
//    }

//    return boxes;
//}

using util::GridBox;

struct Helper {
    Helper(const repres::Representation& r) : repres_(r), latEdges_(/*r.calculateGridBoxLatitudeEdges()*/) {
        ASSERT(latEdges_.size() > 1);
    }

    using edges_t = std::vector<double>;
    using boxes_t = std::vector<GridBox>;

    const edges_t& latitudeEdges() const { return latEdges_; }
    size_t rows() const { return latEdges_.size() - 1; }

    const boxes_t& boxesOnRow(size_t j) {

        // update cache
        preCalculateGridBoxesOnRows({j});

        auto& boxes = boxesOnRows_[j];
        ASSERT(!boxes.empty());
        return boxes;
    }

    std::vector<boxes_t const*> boxesOnRows(const std::vector<size_t>& js) {

        // update cache
        preCalculateGridBoxesOnRows(js);

        std::vector<boxes_t const*> r;
        r.reserve(js.size());

        for (auto j : js) {
            auto& boxes = boxesOnRows_[j];
            ASSERT(!boxes.empty());
            r.emplace_back(&boxes);
        }

        return r;
    }

private:
    const repres::Representation& repres_;
    const edges_t latEdges_;
    std::map<size_t, boxes_t> boxesOnRows_;

    void preCalculateGridBoxesOnRows(const std::vector<size_t>& js) {

        // remove cached, un-requested grid boxes
        for (bool remove = true; remove && !boxesOnRows_.empty();) {
            for (auto& r : boxesOnRows_) {
                if ((remove = !std::count(js.begin(), js.end(), r.first))) {
                    boxesOnRows_.erase(r.first);
                    break;
                }
            }
        }

        // calculate non-cached, requested grid boxes
        for (auto j : js) {
            ASSERT(j < latEdges_.size() - 1);
            GridBox::LatitudeRange lat{latEdges_[j + 1], latEdges_[j]};

            if (!boxesOnRows_.count(j)) {
                std::vector<double> lonEdges(2) /*= repres_.calculateGridBoxLongitudeEdges(j)*/;
                ASSERT(lonEdges.size() > 1);

                auto& boxes = boxesOnRows_[j];
                boxes.reserve(lonEdges.size() - 1);

                for (size_t i = 0; i < lonEdges.size() - 1; ++i) {
                    GridBox::LongitudeRange lon{lonEdges[i], lonEdges[i + 1]};
                    boxes.emplace_back(GridBox{lat, lon});
                }

                ASSERT(!boxes.empty());
            }
        }

        // keep cached only the requested rows
        ASSERT(js.size() == boxesOnRows_.size());
    }
};


GridBoxMethod::GridBoxMethod(const param::MIRParametrisation& parametrisation) : MethodWeighted(parametrisation) {
    if (parametrisation.userParametrisation().has("rotation") ||
        parametrisation.fieldParametrisation().has("rotation")) {
        throw eckit::UserError("GridBoxMethod: rotated input/output not supported");
    }
}


GridBoxMethod::~GridBoxMethod() = default;


bool GridBoxMethod::sameAs(const Method& other) const {
    auto o = dynamic_cast<const GridBoxMethod*>(&other);
    return o && name() == o->name() && MethodWeighted::sameAs(*o);
}


void GridBoxMethod::assemble(util::MIRStatistics&, WeightMatrix& W, const repres::Representation& in,
                             const repres::Representation& out) const {
    eckit::Channel& log = eckit::Log::debug<LibMir>();
    log << "GridBoxMethod::assemble (input: " << in << ", output: " << out << ")" << std::endl;


    if (!in.domain().contains(out.domain())) {
        std::ostringstream msg;
        msg << "GridBoxMethod: input must contain output (input:" << in.domain() << ", output:" << out.domain() << ")";
        throw eckit::UserError(msg.str());
    }


    log << "GridBoxMethod: intersect " << util::Pretty(out.numberOfPoints()) << " from "
        << util::Pretty(in.numberOfPoints(), "grid box", "grid boxes") << std::endl;


    // TODO: triplets, really? why not writing to the matrix directly?
    std::vector<WeightMatrix::Triplet> triplets;
    triplets.reserve(out.numberOfPoints());


    {
        Helper a(in);
        Helper b(out);
        eckit::ProgressTimer progress("Intersecting", a.rows(), "row", 1ul, log);

        auto& aLatEdges = a.latitudeEdges();
        auto& bLatEdges = b.latitudeEdges();

        for (size_t ja = 0; ja < a.rows(); ++ja) {

            // set 'a' LatitudeRange, and 'b' list of LatitudeRanges
            GridBox::LatitudeRange aRow{aLatEdges[ja + 1], aLatEdges[ja]};

            std::vector<size_t> jbs;
            for (size_t jb = 0; jb < b.rows(); ++jb) {
                GridBox::LatitudeRange bRow{bLatEdges[jb + 1], bLatEdges[jb]};
                if (bRow.intersects(aRow)) {
                    jbs.emplace_back(jb);
                }
            }
            ASSERT(!jbs.empty());

            // intersect 'a' boxes (single row) with 'b' boxes (multiple rows)
            auto& aBoxes = a.boxesOnRow(ja);
            for (auto& bBoxes : b.boxesOnRows(jbs)) {

                eckit::Log::info() << "---" << std::endl;
                for (auto& b : aBoxes) {
                    eckit::Log::info() << "a:" << b << std::endl;
                }

                for (auto& b : *bBoxes) {
                    eckit::Log::info() << "b:" << b << std::endl;
                }

                eckit::Log::info() << std::endl;
            }


            ++progress;
        }


#if 0
        for (auto& it : rowIntersect) {
            size_t b = it.first;
            const auto boxesB(boxes({bLatEdges[b + 1], bLatEdges[b]}, out.calculateGridBoxLongitudeEdges(b)));

            // setup input grid boxes on row, and calculate intersections
            for (size_t a : it.second) {
                const auto boxesA(boxes({aLatEdges[a + 1], aLatEdges[a]}, in.calculateGridBoxLongitudeEdges(a)));

                for (size_t colB = 0; colB < boxesB.size(); ++colB) {
                    auto& boxB = boxesB[colB];

                    for (size_t colA = 0; colA < boxesA.size(); ++colA) {
                        auto boxA  = boxesA[colA];  // to intersect (copy)
                        auto areaA = boxA.area();

                        if (boxB.intersects(boxA)) {
                            ASSERT(areaA > 0.);
                            triplets.emplace_back(WeightMatrix::Triplet(colB, colA, boxA.area() / areaA));
                        }
                    }
                }
            }

            ++progress;
        }
#endif
    }


    log << "Intersected " << util::Pretty(triplets.size(), "grid box", "grid boxes") << std::endl;


    // fill sparse matrix
    W.setFromTriplets(triplets);
}


void GridBoxMethod::hash(eckit::MD5& md5) const {
    MethodWeighted::hash(md5);
    md5.add(name());
}


void GridBoxMethod::print(std::ostream& out) const {
    out << "GridBoxMethod["
        << "name=" << name() << ",";
    MethodWeighted::print(out);
    out << "]";
}


}  // namespace gridbox
}  // namespace method
}  // namespace mir
