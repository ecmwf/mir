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
#include "mir/repres/Iterator.h"
#include "mir/util/BoundingBox.h"


namespace mir {
namespace repres {
namespace gauss {


class GaussianIterator : public Iterator {
public:
    typedef std::function<long(size_t)> ni_type;

private:
    const std::vector<double>& latitudes_;
    const util::BoundingBox& bbox_;
    const size_t N_;
    ni_type pl_;
    size_t Ni_;
    size_t Nj_;
    eckit::Fraction lon_;
    eckit::Fraction inc_;
    size_t i_;
    size_t j_;
    size_t k_;
    size_t count_;
    void resetToRow(size_t);
    void setup();

protected:
    void print(std::ostream&) const;
    bool next(Latitude&, Longitude&);

public:

    // Unrotated Gaussian grid iterator
    GaussianIterator(const std::vector<double>& latitudes, const util::BoundingBox&, size_t N, ni_type Ni);

    // Rotated Gaussian grid iterator
    GaussianIterator(const std::vector<double>& latitudes, const util::BoundingBox&, size_t N, ni_type Ni, const util::Rotation&);

    ~GaussianIterator();
};


}  // namespace gauss
}  // namespace repres
}  // namespace mir


#endif

