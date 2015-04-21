/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#ifndef Constrains_H
#define Constrains_H

// namespace src;

// Headers
#ifndef   machine_H
#include "machine.h"
#endif

// Forward declarations
class FieldDescription;


class Constrains {
  public:


// -- Contructors

    Constrains(const string& in, const string& out);

// -- Destructor

    ~Constrains(); // Change to virtual if base class

// -- Operators

    // None

// -- Methods
    void isPossible(FieldDescription* methods, bool global_output) const ;
    void print(ostream&) const; // Change to virtual if base class

// -- Overridden methods
    // None

// -- Class members
    // None

// -- Class methods
    // None


  private:

// No copy allowed

    Constrains(const Constrains&);
    Constrains& operator=(const Constrains&);

// -- Members
    string in_;
    string out_;

// -- Methods
    // None


// -- Friends

    friend ostream& operator<<(ostream& s,const Constrains& p) {
        p.print(s);
        return s;
    }

};

#endif
