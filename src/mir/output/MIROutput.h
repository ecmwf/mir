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

#include <iosfwd>
#include <string>
#include <vector>


namespace mir {
namespace action {
class ActionPlan;
}
namespace context {
class Context;
}
namespace param {
class MIRParametrisation;
}
namespace api {
class MIREstimation;
}
}  // namespace mir


namespace mir::output {


class MIROutput {
public:
    // -- Exceptions
    // None

    // -- Constructors

    MIROutput();
    MIROutput(const MIROutput&) = delete;

    // -- Destructor

    virtual ~MIROutput();

    // -- Convertors
    // None

    // -- Operators

    void operator=(const MIROutput&) = delete;

    // -- Methods

    virtual size_t copy(const param::MIRParametrisation&, context::Context&);
    virtual size_t save(const param::MIRParametrisation&, context::Context&) = 0;
    virtual size_t set(const param::MIRParametrisation&, context::Context&);
    virtual bool sameAs(const MIROutput&) const                                                                = 0;
    virtual bool sameParametrisation(const param::MIRParametrisation&, const param::MIRParametrisation&) const = 0;
    virtual bool printParametrisation(std::ostream&, const param::MIRParametrisation&) const                   = 0;
    virtual void prepare(const param::MIRParametrisation&, action::ActionPlan&, MIROutput&);
    virtual void estimate(const param::MIRParametrisation&, api::MIREstimation&, context::Context&) const;

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

    friend std::ostream& operator<<(std::ostream& s, const MIROutput& p) {
        p.print(s);
        return s;
    }
};


class MIROutputFactory {
    const std::string name_;

    MIROutputFactory(const MIROutputFactory&)            = delete;
    MIROutputFactory& operator=(const MIROutputFactory&) = delete;

protected:
    MIROutputFactory(const std::string& name, const std::vector<std::string>& extensions = {});
    virtual ~MIROutputFactory();
    static const std::vector<std::string> no_extensions;

public:
    virtual MIROutput* make(const std::string& path) = 0;
    static MIROutput* build(const std::string& path, const param::MIRParametrisation&);
    static void list(std::ostream&);
};


template <class T>
class MIROutputBuilder : public MIROutputFactory {
    MIROutput* make(const std::string& path) override { return new T(path); }

public:
    MIROutputBuilder(const std::string& name) : MIROutputFactory(name) {}
    MIROutputBuilder(const std::string& name, const std::vector<std::string>& extensions) :
        MIROutputFactory(name, extensions) {}
};


}  // namespace mir::output
