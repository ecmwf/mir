/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Baudouin Raoult
/// @author Tiago Quintino
/// @author Pedro Maciel
/// @date May 2015


#ifndef mir_method_knn_NearestLSM_h
#define mir_method_knn_NearestLSM_h

#include "mir/method/knn/KNearestNeighbours.h"

#include "mir/method/knn/distance/DistanceWeightingWithLSM.h"


namespace mir {
namespace method {
namespace knn {


class NearestLSM: public KNearestNeighbours {
public:

    NearestLSM(const param::MIRParametrisation&);

    virtual ~NearestLSM();

private:

    virtual void assemble(util::MIRStatistics&, WeightMatrix&, const repres::Representation& in, const repres::Representation& out) const;

    /// Update matrix to account for field masked values
    virtual void applyMasks(WeightMatrix&, const lsm::LandSeaMasks&) const;

    virtual lsm::LandSeaMasks getMasks(const repres::Representation& in, const repres::Representation& out) const;

    virtual const char* name() const;
    virtual bool sameAs(const Method& other) const;

    virtual const pick::Pick& pick() const;
    virtual const distance::DistanceWeighting& distanceWeighting() const;

    std::unique_ptr<const pick::Pick> pick_;
    distance::DistanceWeightingWithLSM distanceWeighting_;

};


}  // namespace knn
}  // namespace method
}  // namespace mir


#endif

