// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include <vector>

#include "mir/output/MIROutput.h"
#include "mir/util/Types.h"


namespace mir::output {


class ValuesOutput : public MIROutput {
public:
    // -- Exceptions
    // None

    // -- Constructors

    ValuesOutput();

    // -- Destructor
    // None

    // -- Convertors
    // None

    // -- Operators
    // None

    // -- Methods

    const MIRValuesVector& values(size_t which = 0) const;
    size_t dimensions() const;

    double missingValue() const;
    bool hasMissing() const;

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

    std::vector<MIRValuesVector> values_;
    double missingValue_;
    bool hasMissing_;

    // -- Methods
    // None

    // -- Overridden methods

    // From MIROutput
    size_t save(const param::MIRParametrisation&, context::Context&) override;
    bool sameAs(const MIROutput&) const override;
    bool sameParametrisation(const param::MIRParametrisation&, const param::MIRParametrisation&) const override;
    bool printParametrisation(std::ostream&, const param::MIRParametrisation&) const override;
    void print(std::ostream&) const override;

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends
    // None
};


}  // namespace mir::output
