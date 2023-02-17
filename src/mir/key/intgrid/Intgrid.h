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


namespace mir::param {
class MIRParametrisation;
}  // namespace mir::param


namespace mir::key::intgrid {


class Intgrid {
public:
    // -- Exceptions
    // None

    // -- Constructors

    Intgrid(const param::MIRParametrisation& parametrisation) : parametrisation_(parametrisation) {}

    Intgrid(const Intgrid&) = delete;

    // -- Destructor

    virtual ~Intgrid() = default;

    // -- Convertors
    // None

    // -- Operators

    void operator=(const Intgrid&) = delete;

    // -- Methods

    virtual const std::string& gridname() const = 0;

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
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends
    // None
};


class IntgridFactory {
    std::string name_;
    virtual Intgrid* make(const param::MIRParametrisation&, long targetGaussianN) = 0;

    IntgridFactory(const IntgridFactory&)            = delete;
    IntgridFactory& operator=(const IntgridFactory&) = delete;

protected:
    IntgridFactory(const std::string&);
    virtual ~IntgridFactory();

public:
    static Intgrid* build(const std::string&, const param::MIRParametrisation&, long targetGaussianN);
    static void list(std::ostream&);
};


template <class T>
class IntgridBuilder : public IntgridFactory {
    Intgrid* make(const param::MIRParametrisation& parametrisation, long targetGaussianN) override {
        return new T(parametrisation, targetGaussianN);
    }

public:
    IntgridBuilder(const std::string& name) : IntgridFactory(name) {}
};


}  // namespace mir::key::intgrid
