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

#include "mir/action/plan/Action.h"


namespace mir::action::filter {


class CesaroSummationFilter : public Action {
public:
    // -- Exceptions
    // None

    // -- Constructors

    CesaroSummationFilter(const param::MIRParametrisation&);
    CesaroSummationFilter(const CesaroSummationFilter&) = delete;

    // -- Destructor

    ~CesaroSummationFilter() override;

    // -- Convertors
    // None

    // -- Operators

    CesaroSummationFilter& operator=(const CesaroSummationFilter&) = delete;

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

    void print(std::ostream&) const override;

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

private:
    // -- Members

    double k_;
    size_t Tmin_;

    // -- Methods
    // None

    // -- Overridden methods

    void execute(context::Context&) const override;
    bool sameAs(const Action&) const override;
    const char* name() const override;
    void estimate(context::Context&, api::MIREstimation&) const override;

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends
    // None
};


}  // namespace mir::action::filter
