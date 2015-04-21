/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#ifndef Field_H
#define Field_H


// Headers
#ifndef   machine_H
#include "machine.h"
#endif

#ifndef Parameter_H
#include "Parameter.h"
#endif

// Forward declarations
class FieldDescription;

//

class Field {
  public:

    // -- Exceptions
    // None

    // -- Contructors

    Field();
    Field(int bitsPerValue, int editionNumber);
    Field(const Parameter &param);

    // Input Field
    Field(const Parameter &param, const string &units, int editionNumber, int centre, const string &levType, int level, int date, int time, const string &stepUnits, int startStep, int endStep, int bitsPerValue);

    // copy general information to output
    // to be del
    Field(const Field &);
    Field(const Field &f, int bitsPerValue, int editionNumber);
    Field(const Parameter &param, const Field &);
    Field(const Parameter &param, const Field &f, int bitsPerValue, int editionNumber) ;

    // Vertical Interpolation
    //	Field(const string& levType, int level, const Field&);

    // -- Destructor

    virtual ~Field(); // Change to virtual if base class

    // -- Convertors
    // None

    // -- Operators
    bool operator==(const Field &other) const;

    // -- Methods
    string         levelType()     const {
        return levelType_;
    }
    int            level()         const {
        return level_;
    }

    Parameter      parameter()     const {
        return parameter_;
    }
    string         units()         const {
        return units_;
    }
    int            centre()        const {
        return centre_;
    }

    int            editionNumber() const {
        return editionNumber_;
    }

    int            date()          const {
        return date_;
    }
    int            time()          const {
        return time_;
    }

    string         stepUnits()     const {
        return stepUnits_;
    }
    int            startStep()     const {
        return startStep_;
    }
    int            endStep()       const {
        return endStep_;
    }
    int            bitsPerValue()  const {
        return bitsPerValue_;
    }

    bool           isSameLevelType( const string &lt) const;
    bool           isSameLevel(int level)             const;

    int           number()        const {
        return parameter_.number();
    }
    int           table()         const {
        return parameter_.table();
    }
    bool           lsm()           const {
        return parameter_.lsm();
    }
    bool           wind()          const {
        return parameter_.wind();
    }
    bool           conservation()  const {
        return parameter_.conservation();
    }
    bool           nearest()       const {
        return parameter_.nearest();
    }

    virtual bool           isRotated()                        const = 0;
    virtual string         nameOfField()                      const = 0;
    virtual void           resetOutput(FieldDescription *out) const = 0;
    virtual FieldDescription *makeOutput(const FieldDescription &out) const = 0;
    virtual const vector<double> &data()                             const = 0;
    virtual size_t dataLength()                       const = 0;
    virtual void   	       dump2file(const string &name)      const = 0;
    virtual void   	       outputArea(double *a)      const = 0;
    virtual void           setOutputAreaAndBasics(FieldDescription &out) const;
    virtual unsigned long           calculatedLength()         const = 0;


  protected:

    // -- Members
    // None

    // -- Methods

    virtual void print(ostream &) const; // Change to virtual if base class

  private:

    // No copy allowed

    Field &operator=(const Field &);

    // -- Members

    Parameter  parameter_;
    string     units_;
    int        centre_;
    int        editionNumber_;

    string     levelType_;
    int        level_;

    int        date_;
    int        time_;

    string     stepUnits_;
    int        startStep_;
    int        endStep_;

    int        bitsPerValue_;

    /*
    int        localDefinitionNumber_;
    */

    // -- Methods
    // None

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends

    friend ostream &operator<<(ostream &s, const Field &p) {
        p.print(s);
        return s;
    }

};

#endif
