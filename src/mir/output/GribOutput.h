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


struct grib_info;
struct grib_handle;


namespace mir::output {


class GribOutput : public MIROutput {
public:
    // -- Exceptions
    // None

    // -- Constructors

    GribOutput();

    // -- Destructor

    ~GribOutput() override;

    // -- Convertors
    // None

    // -- Operators
    // None

    // -- Methods
    virtual size_t interpolated() const;
    virtual size_t saved() const;

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

protected:
    // -- Members

    // -- Methods

    virtual void fill(grib_handle* handle, grib_info& info) const;

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

private:
    // -- Members

    size_t interpolated_;
    size_t saved_;

    // -- Methods

    virtual void out(const void* message, size_t length, bool interpolated) = 0;

    // -- Overridden methods

    // From MIROutput
    size_t copy(const param::MIRParametrisation&, context::Context&) override;
    size_t save(const param::MIRParametrisation&, context::Context&) override;
    size_t set(const param::MIRParametrisation&, context::Context&) override;
    bool sameParametrisation(const param::MIRParametrisation&, const param::MIRParametrisation&) const override;
    bool printParametrisation(std::ostream&, const param::MIRParametrisation&) const override;
    void prepare(const param::MIRParametrisation&, action::ActionPlan&, MIROutput&) override;
    void estimate(const param::MIRParametrisation&, api::MIREstimation&, context::Context&) const override;

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends
    // None
};


}  // namespace mir::output
