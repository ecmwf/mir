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


#ifndef FileLoader_H
#define FileLoader_H

#include "mir/caching/LegendreLoader.h"
#include "eckit/io/Buffer.h"


namespace mir {
namespace caching {


class FileLoader : public LegendreLoader {
  public:

// -- Exceptions
    // None

// -- Contructors

    FileLoader(const param::MIRParametrisation&, const eckit::PathName& path);

// -- Destructor

    ~FileLoader(); // Change to virtual if base class

// -- Convertors
    // None

// -- Operators
    // None

// -- Methods



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

    void print(std::ostream&) const; // Change to virtual if base class

// -- Overridden methods
    // None

// -- Class members
    // None

// -- Class methods
    // None

  private:

// No copy allowed

    FileLoader(const FileLoader&);
    FileLoader& operator=(const FileLoader&);

// -- Members

    eckit::Buffer buffer_;

// -- Methods

// -- Overridden methods

    virtual const void* address() const;
    virtual size_t size() const;


// -- Class members
    // None

// -- Class methods
    // None

// -- Friends

    // friend std::ostream& operator<<(std::ostream& s, const FileLoader& p)
    // { p.print(s); return s; }

};


}  // namespace caching
}  // namespace mir
#endif

