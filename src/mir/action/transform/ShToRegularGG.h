// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include "mir/action/transform/ShToGridded.h"


namespace mir::action::transform {


template <class Invtrans>
class ShToRegularGG : public ShToGridded, Invtrans {
public:
    // -- Exceptions
    // None

    // -- Constructors

    explicit ShToRegularGG(const param::MIRParametrisation&);

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

    size_t N_;

    // -- Methods
    // None

    // -- Overridden methods

    bool sameAs(const Action&) const override;
    const char* name() const override;
    const repres::Representation* outputRepresentation() const override;
    void print(std::ostream&) const override;
    void sh2grid(data::MIRField& field, const atlas_trans_t& trans, const param::MIRParametrisation&) const override;

    bool getGriddedTargetName(std::string& name) const override;

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends
    // None
};


}  // namespace mir::action::transform
