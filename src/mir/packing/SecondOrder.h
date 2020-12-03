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


#ifndef mir_packing_SecondOrder_h
#define mir_packing_SecondOrder_h

#include "mir/packing/Packer.h"


namespace mir {
namespace packing {


class SecondOrder : public Packer {
public:
    // -- Types
    // None

    // -- Exceptions
    // None

    // -- Constructors

    SecondOrder(const std::string& name);

    // -- Destructor

    virtual ~SecondOrder() override;

    // -- Convertors
    // None

    // -- Operators
    // None

    // -- Methods
    // None

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

protected:
    // -- Members
    // None

    // -- Methods
    // None

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

private:
    // -- Members
    // None

    // -- Methods

    virtual void print(std::ostream&) const override;
    virtual void fill(grib_info&, const repres::Representation&, const param::MIRParametrisation&,
                      const param::MIRParametrisation&) const;

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends
    // None
};


}  // namespace packing
}  // namespace mir


#endif
