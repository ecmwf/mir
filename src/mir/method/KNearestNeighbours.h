/*
 * (C) Copyright 1996-2015 ECMWF.
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


#ifndef mir_method_KNearestNeighbours_h
#define mir_method_KNearestNeighbours_h

#include <string>
#include "mir/method/MethodWeighted.h"


namespace mir {
namespace method {
namespace distance {
class DistanceWeighting;
}
}
}


namespace mir {
namespace method {

//----------------------------------------------------------------------------------------------------------------------

class KNearestNeighbours : public MethodWeighted {
public:

    explicit KNearestNeighbours(const param::MIRParametrisation&);

    virtual ~KNearestNeighbours();

    virtual void hash(eckit::MD5&) const;

protected:

    void assemble(
            util::MIRStatistics&,
            WeightMatrix&,
            const repres::Representation& in,
            const repres::Representation& out,
            const distance::DistanceWeighting& ) const;

    virtual void assemble(
            util::MIRStatistics&,
            WeightMatrix&,
            const repres::Representation& in,
            const repres::Representation& out ) const;

private:

    virtual void print(std::ostream&) const;

    virtual const char *name() const = 0;

    virtual size_t nClosest() const;

    virtual std::string distanceWeighting() const;

};

//----------------------------------------------------------------------------------------------------------------------

}  // namespace method
}  // namespace mir

#endif
