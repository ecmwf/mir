/*
 * (C) Copyright 1996-2017 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation
 * nor does it submit to any jurisdiction.
 */

/// @date September 2017


#ifndef mir_style_SpectralMode_h
#define mir_style_SpectralMode_h

#include <iosfwd>
#include <string>
#include "eckit/memory/NonCopyable.h"
#include "mir/param/MIRParametrisation.h"


namespace mir {
namespace style {


class SpectralMode : public eckit::NonCopyable {
public:

    // -- Exceptions
    // None

    // -- Contructors

    SpectralMode(const param::MIRParametrisation&);

    // -- Destructor

    virtual ~SpectralMode();

    // -- Convertors
    // None

    // -- Operators
    // None

    // -- Methods

    virtual bool getGridname(std::string&) const = 0;
    virtual long getTruncation() const = 0;

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


class SpectralModeFactory {
    std::string name_;
    virtual SpectralMode *make(const param::MIRParametrisation&) = 0;
protected:
    SpectralModeFactory(const std::string&);
    virtual ~SpectralModeFactory();
public:
    static SpectralMode *build(const std::string&, const param::MIRParametrisation&);
    static void list(std::ostream&);
};


template <class T> class SpectralModeBuilder : public SpectralModeFactory {
    virtual SpectralMode *make(const param::MIRParametrisation& p) {
        return new T(p);
    }
public:
    SpectralModeBuilder(const std::string& name) : SpectralModeFactory(name) {}
};


}  // namespace style
}  // namespace mir


#endif
