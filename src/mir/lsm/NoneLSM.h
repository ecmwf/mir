// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include "mir/lsm/LSMSelection.h"


namespace mir::lsm {


class NoneLSM : public LSMSelection {
public:
    // -- Exceptions
    // None

    // -- Constructors

    NoneLSM(const std::string& name);

    // -- Destructor
    // None

    // -- Convertors
    // None

    // -- Operators
    // None

    // -- Methods

    static Mask& noMask();

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

    void print(std::ostream&) const override;

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
    // None

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods

    Mask* create(const param::MIRParametrisation&, const repres::Representation&,
                 const std::string& which) const override;

    std::string cacheKey(const param::MIRParametrisation&, const repres::Representation&,
                         const std::string& which) const override;

    virtual std::string cacheName() const;

    // -- Friends
    // None
};


}  // namespace mir::lsm
