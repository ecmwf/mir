// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include <iosfwd>
#include <string>


namespace mir {
namespace context {
class Context;
}
namespace param {
class MIRParametrisation;
}
namespace action {
class ActionNode;
}
}  // namespace mir


namespace mir::action {
class Executor {
public:
    // -- Exceptions
    // None

    // -- Constructors

    Executor(const Executor&) = delete;
    Executor(Executor&&)      = delete;

    // -- Destructor
    // None

    // -- Convertors
    // None

    // -- Operators

    void operator=(const Executor&) = delete;
    void operator=(Executor&&)      = delete;

    // -- Methods

    virtual void execute(context::Context&, const ActionNode&) const = 0;
    virtual void wait() const                                        = 0;
    virtual void parametrisation(const param::MIRParametrisation&)   = 0;

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods

    static const Executor& lookup(const param::MIRParametrisation&);
    static void list(std::ostream&, bool full = false);

protected:
    Executor(const std::string&);
    virtual ~Executor();

    // -- Members

    std::string name_;

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

    friend std::ostream& operator<<(std::ostream& s, const Executor& p) {
        p.print(s);
        return s;
    }
};


}  // namespace mir::action
