/*
 * (C) Copyright 1996-2017 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation
 * nor does it submit to any jurisdiction.
 */

/// @date May 2017


#ifndef mir_style_IntermediateGrid_h
#define mir_style_IntermediateGrid_h

#include <iosfwd>
#include <string>
#include "eckit/memory/NonCopyable.h"
#include "mir/param/DelayedParametrisation.h"
#include "mir/param/MIRParametrisation.h"


namespace mir {
namespace style {


class IntermediateGrid: public eckit::NonCopyable, public param::DelayedParametrisation {
public:

    // -- Exceptions
    // None

    // -- Contructors

    IntermediateGrid(const param::MIRParametrisation&);

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

    const param::MIRParametrisation& parametrisation_;

    // -- Methods

    virtual std::string getGridname() const = 0;

    // -- Overridden methods

    bool get(const std::string&, std::string&) const;

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


class IntermediateGridFactory {
    std::string name_;
    virtual IntermediateGrid *make(const param::MIRParametrisation&) = 0;
protected:
    IntermediateGridFactory(const std::string&);
    virtual ~IntermediateGridFactory();
public:
    static IntermediateGrid *build(const std::string&, const param::MIRParametrisation&);
    static void list(std::ostream&);
};


template <class T> class IntermediateGridBuilder : public IntermediateGridFactory {
    virtual IntermediateGrid *make(const param::MIRParametrisation& p) {
        return new T(p);
    }
public:
    IntermediateGridBuilder(const std::string& name) : IntermediateGridFactory(name) {}
};


}  // namespace style
}  // namespace mir


#endif
