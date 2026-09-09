// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include <vector>

#include "mir/action/transform/ShToGridded.h"
#include "mir/util/Rotation.h"


namespace mir::action::transform {


template <class Invtrans>
class ShToRotatedReducedGGPLGiven : public ShToGridded, Invtrans {
public:
    // -- Exceptions
    // None

    // -- Constructors

    explicit ShToRotatedReducedGGPLGiven(const param::MIRParametrisation&);

    // -- Destructor
    // None

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

private:
    // -- Members

    std::vector<long> pl_;
    util::Rotation rotation_;

    // -- Methods
    // None

    // -- Overridden methods

    bool sameAs(const Action&) const override;
    const char* name() const override;
    const repres::Representation* outputRepresentation() const override;
    void print(std::ostream&) const override;
    void sh2grid(data::MIRField& field, const atlas_trans_t& trans, const param::MIRParametrisation&) const override;

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends
    // None
};


}  // namespace mir::action::transform
