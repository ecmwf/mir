/*
 * (C) Copyright 1996-2017 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation
 * nor does it submit to any jurisdiction.
 */

/// @date May 2017


#ifndef mir_style_IntermediateGaussianGrid_h
#define mir_style_IntermediateGaussianGrid_h

#include "eckit/exception/Exceptions.h"
#include "eckit/memory/ScopedPtr.h"
#include "mir/action/transform/mapping/Mapping.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/style/IntermediateGrid.h"


namespace mir {
namespace style {


template< typename GRIDTYPE >
class IntermediateGaussianGrid: public IntermediateGrid {
public:

    // -- Exceptions
    // None

    // -- Contructors

    IntermediateGaussianGrid(const param::MIRParametrisation& parametrisation) :
        IntermediateGrid(parametrisation) {

        mapping_ = "linear";
        parametrisation_.get("spectral-mapping", mapping_);

        using namespace action::transform::mapping;
        eckit::ScopedPtr<Mapping> map(MappingFactory::build(mapping_));
        ASSERT(map);

        truncation_ = 0;
        ASSERT(parametrisation_.get("truncation", truncation_));

        long Ni = map->getPointsPerLatitudeFromTruncation(long(truncation_));
        ASSERT(Ni > 0);

        N_ = size_t(Ni/4L);
        ASSERT(N_);
    }

    // -- Destructor
    // None

    // -- Convertors
    // None

    // -- Operators
    // None

    // -- Methods

    size_t gaussianNumber() const {
        return N_;
    }

    std::string gaussianGridType() const {
        std::ostringstream os;
        os << "IntermediateGaussianGrid::gaussianGridType() not implemented for " << *this;
        throw eckit::SeriousBug(os.str());
    }

    // -- Overridden methods

    void print(std::ostream& out) const {
        out << "IntermediateGaussianGrid["
                "mapping=" << mapping_
            << ",truncation=" << truncation_
            << ",gridType=" << gaussianGridType()
            << ",N=" << gaussianNumber()
            << "]";
    }

    // -- Class members
    // None

    // -- Class methods
    // None

private:

    // -- Members

    std::string mapping_;
    size_t truncation_;
    size_t N_;

    // -- Methods
    // None

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends
    // None

};


}  // namespace style
}  // namespace mir


#endif
