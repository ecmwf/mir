// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include <iosfwd>
#include <string>


namespace mir {
namespace action {
class ActionPlan;
}
namespace output {
class MIROutput;
}
namespace param {
class MIRParametrisation;
}
}  // namespace mir


namespace mir::key::style {


class MIRStyle {
public:
    // -- Exceptions
    // None

    // -- Constructors

    MIRStyle(const param::MIRParametrisation&);
    MIRStyle(const MIRStyle&) = delete;

    // -- Destructor

    virtual ~MIRStyle();

    // -- Convertors
    // None

    // -- Operators

    void operator=(const MIRStyle&) = delete;

    // -- Methods

    virtual void prepare(action::ActionPlan&, output::MIROutput&) const = 0;

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

protected:
    // -- Members

    const param::MIRParametrisation& parametrisation_;

    // -- Methods

    virtual void print(std::ostream&) const = 0;

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
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends

    friend std::ostream& operator<<(std::ostream& s, const MIRStyle& p) {
        p.print(s);
        return s;
    }
};


class MIRStyleFactory {
    std::string name_;
    virtual MIRStyle* make(const param::MIRParametrisation&) = 0;

protected:
    MIRStyleFactory(const std::string&);
    virtual ~MIRStyleFactory();

public:
    static MIRStyle* build(const param::MIRParametrisation&);
    static void list(std::ostream&);

    MIRStyleFactory(const MIRStyleFactory&)            = delete;
    MIRStyleFactory(MIRStyleFactory&&)                 = delete;
    MIRStyleFactory& operator=(const MIRStyleFactory&) = delete;
    MIRStyleFactory& operator=(MIRStyleFactory&&)      = delete;
};


template <class T>
class MIRStyleBuilder : public MIRStyleFactory {
    MIRStyle* make(const param::MIRParametrisation& param) override { return new T(param); }

public:
    MIRStyleBuilder(const std::string& name) : MIRStyleFactory(name) {}
};


}  // namespace mir::key::style
