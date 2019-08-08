/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Tiago Quintino
/// @author Pedro Maciel
/// @date May 2015


#ifndef mir_method_knn_KNearest_h
#define mir_method_knn_KNearest_h

#include "mir/method/knn/KNearestNeighbours.h"
#include "mir/method/knn/pick/Pick.h"


namespace mir {
namespace method {
namespace knn {


class KNearest : public KNearestNeighbours {
public:
    KNearest(const param::MIRParametrisation&);

    virtual ~KNearest();

private:
    virtual void assemble(util::MIRStatistics&, WeightMatrix&, const repres::Representation& in,
                          const repres::Representation& out) const;

    virtual const char* name() const;
    virtual bool sameAs(const Method&) const;

    void print(std::ostream&) const;

    const pick::Pick& pick() const;
    std::unique_ptr<const pick::Pick> pick_;
};


}  // namespace knn
}  // namespace method
}  // namespace mir


#endif
