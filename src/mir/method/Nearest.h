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


#ifndef mir_method_Nearest_H
#define mir_method_Nearest_H

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

class Nearest : public MethodWeighted {
public:

    explicit Nearest(const param::MIRParametrisation&);

    virtual ~Nearest();

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

    virtual size_t nclosest() const = 0;

    virtual std::string distanceWeighting() const = 0;

};

//----------------------------------------------------------------------------------------------------------------------

}  // namespace method
}  // namespace mir

#endif
