// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include <vector>

#include "mir/output/MIROutput.h"
#include "mir/param/SimpleParametrisation.h"


namespace mir::output {


class ResizableOutput final : public MIROutput {
public:
    // -- Exceptions
    // None

    // -- Constructors

    ResizableOutput(std::vector<double>&, param::SimpleParametrisation& metadata);

    // -- Destructor
    // None

    // -- Convertors
    // None

    // -- Operators
    // None

    // -- Methods

    size_t size() const;

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

private:
    // -- Members

    std::vector<double>& values_;
    param::SimpleParametrisation& metadata_;

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
