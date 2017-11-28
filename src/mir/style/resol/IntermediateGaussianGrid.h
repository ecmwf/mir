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


#ifndef mir_style_resol_IntermediateGaussianGrid_h
#define mir_style_resol_IntermediateGaussianGrid_h

#include "eckit/exception/Exceptions.h"
#include "eckit/memory/ScopedPtr.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/style/resol/IntermediateGrid.h"
#include "mir/style/resol/SpectralOrder.h"


namespace mir {
namespace style {
namespace resol {


template< typename GRIDTYPE >
class IntermediateGaussianGrid : public IntermediateGrid {
public:

    // -- Exceptions
    // None

    // -- Contructors

    IntermediateGaussianGrid(const param::MIRParametrisation& parametrisation) : IntermediateGrid(parametrisation) {

        // Setup spectral order mapping
        std::string order;
        parametrisation_.get("spectral-order", order);

        eckit::ScopedPtr<resol::SpectralOrder> spectralOrder(resol::SpectralOrderFactory::build(order));
        ASSERT(spectralOrder);

        long T = 0;
        ASSERT(parametrisation_.get("spectral", T));
        ASSERT(T > 0);

        long N = spectralOrder->getGaussianNumberFromTruncation(T);
        ASSERT(N > 0);

        gridname_ = gaussianGridTypeLetter() + std::to_string(N);
    }

    // -- Destructor
    // None

    // -- Convertors
    // None

    // -- Operators
    // None

    // -- Methods
    // None

    // -- Overridden methods

    // -- Class members
    // None

    // -- Class methods
    // None

private:

    // -- Members

    std::string gridname_;

    // -- Methods

    std::string gaussianGridTypeLetter() const {
        std::ostringstream os;
        os << "IntermediateGaussianGrid::gaussianGridTypeLetter() not implemented for " << *this;
        throw eckit::SeriousBug(os.str());
    }

    // -- Overridden methods

    std::string getGridname() const {
        return gridname_;
    }

    void print(std::ostream& out) const {
        out << "IntermediateGaussianGrid[gridname=" << gridname_ << "]";
    }

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends
    // None

};


}  // namespace resol
}  // namespace style
}  // namespace mir


#endif
