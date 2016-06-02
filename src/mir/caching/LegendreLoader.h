/*
 * (C) Copyright 1996-2015 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Baudouin Raoult
/// @author Pedro Maciel
/// @date Apr 2015


#ifndef LegendreLoader_H
#define LegendreLoader_H

#include <iosfwd>
// #include <memory>
// #include <string>
// #include <vector>

#include "eckit/filesystem/PathName.h"

namespace mir {


namespace param {
class MIRParametrisation;
}

namespace caching {


class LegendreLoader {
  public:

// -- Exceptions
    // None

// -- Contructors

    LegendreLoader(const param::MIRParametrisation& parametrisation, const eckit::PathName&);

// -- Destructor

    virtual ~LegendreLoader(); // Change to virtual if base class

// -- Convertors
    // None

// -- Operators
    // None

// -- Methods

    virtual const void* address() const = 0;
    virtual size_t size() const = 0;

// -- Overridden methods
    // None

// -- Class members
    // None

// -- Class methods
    // None

  protected:

// -- Members

    const param::MIRParametrisation& parametrisation_;
    eckit::PathName path_;

// -- Methods


    virtual void print(std::ostream&) const = 0; // Change to virtual if base class

// -- Overridden methods
    // None

// -- Class members
    // None

// -- Class methods
    // None

  private:

// No copy allowed

    LegendreLoader(const LegendreLoader&);
    LegendreLoader& operator=(const LegendreLoader&);

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

    friend std::ostream& operator<<(std::ostream& s,const LegendreLoader& p) {
        p.print(s);
        return s;
    }

};


class LegendreLoaderFactory {
    std::string name_;
    virtual LegendreLoader* make(const param::MIRParametrisation&, const eckit::PathName& path) = 0 ;

  protected:

    LegendreLoaderFactory(const std::string&);
    virtual ~LegendreLoaderFactory();

  public:
    static LegendreLoader* build(const param::MIRParametrisation&, const eckit::PathName& path);
    static void list(std::ostream &);

};


template<class T>
class LegendreLoaderBuilder : public LegendreLoaderFactory {
    virtual LegendreLoader* make(const param::MIRParametrisation& param, const eckit::PathName& path) {
        return new T(param, path);
    }
  public:
    LegendreLoaderBuilder(const std::string& name) : LegendreLoaderFactory(name) {}
};


}  // namespace caching
}  // namespace mir
#endif

