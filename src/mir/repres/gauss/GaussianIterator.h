/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#ifndef mir_repres_gauss_reduced_GaussianIterator_h
#define mir_repres_gauss_reduced_GaussianIterator_h

#include <functional>
#include "eckit/types/Fraction.h"
#include "mir/util/Domain.h"


namespace mir {
namespace repres {
namespace gauss {


class GaussianIterator {
public:
    typedef std::function<long(size_t)> pl_type;
private:
    const std::vector<double>& latitudes_;
    const util::Domain domain_;
    const size_t N_;
    const size_t countTotal_;
    pl_type pl_;
    size_t Ni_;
    size_t Nj_;
    const eckit::Fraction west_;
    eckit::Fraction lon_;
    eckit::Fraction inc_;
    size_t i_;
    size_t j_;
    size_t k_;
    size_t count_;
protected:
    ~GaussianIterator();
    void print(std::ostream&) const;
    bool next(Latitude&, Longitude&);
    virtual long Nj(size_t i) = 0;
public:
    GaussianIterator(const std::vector<double>& latitudes, const util::Domain&, size_t N, size_t countTotal, pl_type pl);
};


}  // namespace gauss
}  // namespace repres
}  // namespace mir


#endif

