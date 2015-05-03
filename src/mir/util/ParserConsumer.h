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


#ifndef ParserConsumer_H
#define ParserConsumer_H

#include <string>



namespace mir {
namespace util {

class ParserConsumer  {
  public:

// -- Exceptions
    // None

// -- Contructors

    ParserConsumer();

// -- Destructor

    virtual ~ParserConsumer(); // Change to virtual if base class

// -- Convertors
    // None

// -- Operators
    // None

// -- Methods

    virtual void set(const std::string& name, const char* value) = 0;
    virtual void set(const std::string& name, const std::string& value) = 0;
    virtual void set(const std::string& name, bool value) = 0;
    virtual void set(const std::string& name, long value) = 0;
    virtual void set(const std::string& name, double value) = 0;

// -- Overridden methods
    // None

// -- Class members
    // None

// -- Class methods
    // None

  protected:

// -- Members



// -- Methods

    // virtual void print(std::ostream&) const = 0; // Change to virtual if base class


// -- Overridden methods


// -- Class members
    // None

// -- Class methods
    // None

  private:

// No copy allowed

    ParserConsumer(const ParserConsumer&);
    ParserConsumer& operator=(const ParserConsumer&);

// -- Members


// -- Methods
    // None

// -- Overridden methods
    // None



// -- Class members
    // None

// -- Class methods
    // None

// -- Friends


    // friend std::ostream& operator<<(std::ostream& s,const ParserConsumer& p) {
    //     p.print(s);
    //     return s;
    // }

};


}  // namespace param
}  // namespace mir
#endif

