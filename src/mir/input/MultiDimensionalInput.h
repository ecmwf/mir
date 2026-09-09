// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include <vector>

#include "mir/input/MIRInput.h"


namespace mir::output {
class MultiDimensionalOutput;
}  // namespace mir::output


namespace mir::input {


class MultiDimensionalInput : public MIRInput {
public:
    // -- Exceptions
    // None

    // -- Constructors

    MultiDimensionalInput();

    // -- Destructor

    ~MultiDimensionalInput() override;

    // -- Convertors
    // None

    // -- Operators
    // None

    // -- Methods

    void append(MIRInput*);

    // -- Overridden methods

    size_t dimensions() const override;

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

    std::vector<MIRInput*> dimensions_;

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
    void setAuxiliaryInformation(const util::ValueMap&) override;

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends

    friend class output::MultiDimensionalOutput;
};


}  // namespace mir::input
