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


#ifndef mir_method_knn_NearestNeighbour_h
#define mir_method_knn_NearestNeighbour_h

#include "mir/method/knn/KNearestNeighbours.h"

#include "mir/method/knn/distance/InverseDistanceWeightingSquared.h"


namespace mir {
namespace method {
namespace knn {


class NearestNeighbour : public KNearestNeighbours {
public:
    NearestNeighbour(const param::MIRParametrisation&);

    virtual ~NearestNeighbour() override;

private:
    virtual const char* name() const;
    virtual bool sameAs(const Method& other) const;
    virtual const pick::Pick& pick() const;
    virtual const distance::DistanceWeighting& distanceWeighting() const;

    std::unique_ptr<const pick::Pick> pick_;
    distance::InverseDistanceWeightingSquared distanceWeighting_;
};


}  // namespace knn
}  // namespace method
}  // namespace mir


#endif
