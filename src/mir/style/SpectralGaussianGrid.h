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


#ifndef mir_style_SpectralGaussianGrid_h
#define mir_style_SpectralGaussianGrid_h

#include "eckit/exception/Exceptions.h"
#include "eckit/memory/ScopedPtr.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/style/SpectralGrid.h"
#include "mir/style/SpectralOrder.h"


namespace mir {
namespace style {


template< typename GRIDTYPE >
class SpectralGaussianGrid : public SpectralGrid {
public:

    // -- Exceptions
    // None

    // -- Contructors

    SpectralGaussianGrid(const param::MIRParametrisation& parametrisation) : SpectralGrid(parametrisation) {

        order_ = "linear";
        parametrisation_.get("spectral-order", order_);
        eckit::ScopedPtr<SpectralOrder> order(SpectralOrderFactory::build(order_));
        ASSERT(order);

        truncation_ = 0;
        ASSERT(parametrisation_.get("truncation", truncation_));
        long N = order->getGaussianNumberFromTruncation(long(truncation_));
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

    size_t truncation_;
    std::string order_;
    std::string gridname_;

    // -- Methods

    std::string gaussianGridTypeLetter() const {
        std::ostringstream os;
        os << "SpectralGaussianGrid::gaussianGridTypeLetter() not implemented for " << *this;
        throw eckit::SeriousBug(os.str());
    }

    // -- Overridden methods

    bool active() const {
        return true;
    }

    std::string getGridname() const {
        return gridname_;
    }

    void print(std::ostream& out) const {
        out << "SpectralGaussianGrid["
               "truncation=" << truncation_
            << ",order=" << order_
            << ",gridname=" << gridname_
            << "]";
    }

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
