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


#pragma once

#include "mir/output/MIROutput.h"


namespace mir::output {


class VectorOutput : public MIROutput {
public:
    // -- Exceptions
    // None

    // -- Constructors

    VectorOutput(MIROutput& component1, MIROutput& component2);

    // -- Destructor

    ~VectorOutput() override;

    // -- Convertors
    // None

    // -- Operators
    // None

    // -- Methods
    // None

    // -- Overridden methods

    // From MIROutput
    size_t copy(const param::MIRParametrisation&, context::Context&) override;
    size_t save(const param::MIRParametrisation&, context::Context&) override;
    size_t set(const param::MIRParametrisation&, context::Context&) override;

    // -- Class members
    // None

    // -- Class methods
    // None

protected:
    // -- Members

    MIROutput& component1_;
    MIROutput& component2_;

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

    // From MIROutput
    bool sameAs(const MIROutput&) const override;
    bool sameParametrisation(const param::MIRParametrisation&, const param::MIRParametrisation&) const override;
    bool printParametrisation(std::ostream&, const param::MIRParametrisation&) const override;
    void prepare(const param::MIRParametrisation&, action::ActionPlan&, MIROutput&) override;
    void print(std::ostream&) const override;

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends
    // None
};


}  // namespace mir::output
