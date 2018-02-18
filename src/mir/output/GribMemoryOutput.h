/*
 * (C) Copyright 1996- ECMWF.
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


#ifndef GribMemoryOutput_H
#define GribMemoryOutput_H

#include "mir/output/GribOutput.h"


namespace mir {
namespace output {


class GribMemoryOutput : public GribOutput {
  public:

    // -- Exceptions
    // None

    // -- Contructors

    GribMemoryOutput(void *message, size_t size);

    // -- Destructor

    virtual ~GribMemoryOutput(); // Change to virtual if base class

    // -- Convertors
    // None

    // -- Operators
    // None

    // -- Methods
    // None

    size_t interpolated() const {
        return interpolated_;
    }
    size_t saved() const {
        return saved_;
    }
    size_t length() const {
        return length_;
    }


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


    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

  private:

    // -- Members

    void* message_;
    size_t size_;
    size_t length_;

    size_t saved_;
    size_t interpolated_;

    // -- Methods

    // -- Overridden methods

    virtual void out(const void* message, size_t length, bool iterpolated);
    virtual void print(std::ostream&) const;
    virtual bool sameAs(const MIROutput& other) const;

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends

    //friend ostream& operator<<(ostream& s,const GribMemoryOutput& p)
    //  { p.print(s); return s; }

};


}  // namespace output
}  // namespace mir
#endif

