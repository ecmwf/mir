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


#pragma once

#include "mir/method/knn/KNearestNeighbours.h"

#include <memory>


namespace mir {
namespace method {
namespace knn {


class KNearest : public KNearestNeighbours {
public:
    KNearest(const param::MIRParametrisation&);

    ~KNearest() override;

private:
    const char* name() const override;
    bool sameAs(const Method&) const override;
    const pick::Pick& pick() const override;
    const distance::DistanceWeighting& distanceWeighting() const override;

    std::unique_ptr<const pick::Pick> pick_;
    std::unique_ptr<const distance::DistanceWeighting> distanceWeighting_;
};


}  // namespace knn
}  // namespace method
}  // namespace mir
