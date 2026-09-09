// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include <string>

#include "mir/action/plan/Action.h"
#include "mir/util/MeshGeneratorParameters.h"


namespace mir::action {


class NablaFilter : public Action {
public:
    // -- Exceptions
    // None

    // -- Constructors

    NablaFilter(const param::MIRParametrisation&);

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

protected:
    // -- Members

    util::MeshGeneratorParameters meshGeneratorParams_;
    bool polesMissingValues_;

    // -- Methods
    // None

    // -- Overridden methods

    void print(std::ostream&) const override;
    void custom(std::ostream&) const override;

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
    // None

    // -- Friends
    // None
};


class NablaFilterFactory : public ActionFactory {
protected:
    NablaFilterFactory(const std::string& name);
    ~NablaFilterFactory() override;

public:
    static void list(std::ostream&);
};


template <class T>
class NablaFilterBuilder : public NablaFilterFactory {
    NablaFilter* make(const param::MIRParametrisation& param) override { return new T(param); }

public:
    NablaFilterBuilder(const std::string& name) : NablaFilterFactory(name) {}
};


}  // namespace mir::action
