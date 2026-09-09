// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include "mir/input/MIRInput.h"


namespace mir::output {
class VectorOutput;
}  // namespace mir::output


namespace mir::input {


class VectorInput : public MIRInput {
public:
    // -- Exceptions
    // None

    // -- Constructors

    VectorInput(MIRInput& component1, MIRInput& component2);

    // -- Destructor

    ~VectorInput() override;

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

    MIRInput& component1_;
    MIRInput& component2_;

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
    // None

    // -- Overridden methods

    // From MIRInput
    const param::MIRParametrisation& parametrisation(size_t which) const override;
    data::MIRField field() const override;
    bool next() override;
    bool sameAs(const MIRInput&) const override;
    void print(std::ostream&) const override;
    grib_handle* gribHandle(size_t which = 0) const override;
    size_t dimensions() const override;

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends

    friend class output::VectorOutput;
};


}  // namespace mir::input
