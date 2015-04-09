// File DummyInput.h
// Baudouin Raoult - (c) ECMWF Apr 15

#ifndef DummyInput_H
#define DummyInput_H

// namespace outline;


#include "MIRInput.h"
#include "FieldParametrisation.h"

#include <map>
#include <string>


class DummyInput : public MIRInput, public FieldParametrisation {
  public:

    // -- Exceptions
    // None

    // -- Contructors
    DummyInput();

    // -- Destructor

    virtual ~DummyInput(); // Change to virtual if base class

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
    // None

    // -- Methods


    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

  private:

    // No copy allowed

    DummyInput(const DummyInput &);
    DummyInput &operator=(const DummyInput &);

    // -- Members

    std::map<std::string, std::string> settings_;


    // -- Methods

    // -- Overridden methods
    // From MIRInput

    virtual void print(std::ostream&) const; // Change to virtual if base class

    virtual const MIRParametrisation &parametrisation() const;
    virtual MIRField *field() const;

    // From FieldParametrisation
    virtual bool lowLevelGet(const std::string&, std::string&) const;

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends

    //friend ostream& operator<<(ostream& s,const DummyInput& p)
    //  { p.print(s); return s; }

};

#endif
