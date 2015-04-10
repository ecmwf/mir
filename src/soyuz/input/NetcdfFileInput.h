// File NetcdfFileInput.h
// Baudouin Raoult - (c) ECMWF Apr 15

#ifndef NetcdfFileInput_H
#define NetcdfFileInput_H

// namespace outline;


#include "soyuz/input/MIRInput.h"
#include "soyuz/param/FieldParametrisation.h"
#include "eckit/filesystem/PathName.h"


#include <string>

class NetcdfFileInput : public MIRInput, public FieldParametrisation {
public:

    // -- Exceptions
    // None

    // -- Contructors

    NetcdfFileInput(const eckit::PathName&, const std::string& variable);

    // -- Destructor

    virtual ~NetcdfFileInput(); // Change to virtual if base class

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

    NetcdfFileInput(const NetcdfFileInput &);
    NetcdfFileInput &operator=(const NetcdfFileInput &);

    // -- Members

    eckit::PathName path_;
    std::string variable_;
    mutable int nc_;
    mutable std::vector<double> latitude_;
    mutable std::vector<double> longitude_;

    // -- Methods

    void getVariable(const std::string& name, std::vector<double>& values) const;

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

    //friend ostream& operator<<(ostream& s,const NetcdfFileInput& p)
    //  { p.print(s); return s; }

};

#endif
