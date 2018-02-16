/*
 * (C) Copyright 1996-2017 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation
 * nor does it submit to any jurisdiction.
 */


#ifndef mir_style_Intgrid_h
#define mir_style_Intgrid_h

#include <iosfwd>
#include <string>
#include "eckit/memory/NonCopyable.h"


namespace mir {
namespace param {
class MIRParametrisation;
}
}


namespace mir {
namespace style {


class Intgrid : public eckit::NonCopyable {
public:

    // -- Exceptions
    // None

    // -- Contructors

    Intgrid(const param::MIRParametrisation& parametrisation) :
        parametrisation_(parametrisation) {
    }

    // -- Destructor

    virtual ~Intgrid() {}

    // -- Convertors
    // None

    // -- Operators
    // None

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
protected:
    IntgridFactory(const std::string&);
    virtual ~IntgridFactory();
public:
    static Intgrid* build(const std::string&, const param::MIRParametrisation&, long targetGaussianN);
    static void list(std::ostream&);
};


template <class T> class IntgridBuilder : public IntgridFactory {
    virtual Intgrid* make(const param::MIRParametrisation& parametrisation, long targetGaussianN) {
        return new T(parametrisation, targetGaussianN);
    }
public:
    IntgridBuilder(const std::string& name) : IntgridFactory(name) {}
};


}  // namespace style
}  // namespace mir


#endif
