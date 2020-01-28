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


#ifndef mir_method_knn_KNearestNeighbours_h
#define mir_method_knn_KNearestNeighbours_h

#include <memory>

#include "mir/method/MethodWeighted.h"


namespace mir {
namespace method {
namespace knn {
namespace distance {
class DistanceWeighting;
}
namespace pick {
class Pick;
}
}
}
}


namespace mir {
namespace method {
namespace knn {


class KNearestNeighbours : public MethodWeighted {
public:

    explicit KNearestNeighbours(const param::MIRParametrisation&);

    virtual ~KNearestNeighbours();

    virtual void hash(eckit::MD5&) const;

protected:
    void assemble(util::MIRStatistics&, WeightMatrix&, const repres::Representation& in,
                  const repres::Representation& out, const pick::Pick& pick, const distance::DistanceWeighting&) const;

    virtual void assemble(util::MIRStatistics&, WeightMatrix&, const repres::Representation& in,
                          const repres::Representation& out) const;

    virtual bool sameAs(const Method& other) const = 0;

private:

    virtual void print(std::ostream&) const;

    virtual bool canIntroduceMissingValues() const;

    virtual const char *name() const = 0;

    virtual const pick::Pick& pick() const                               = 0;
    virtual const distance::DistanceWeighting& distanceWeighting() const = 0;
};


}  // namespace knn
}  // namespace method
}  // namespace mir


#endif

