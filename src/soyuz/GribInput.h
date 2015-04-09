// File GribInput.h
// Baudouin Raoult - (c) ECMWF Apr 15

#ifndef GribInput_H
#define GribInput_H

// namespace outline;


#include "MIRInput.h"
#include "MIRParametrisation.h"

#include <memory>

class GribInput : public MIRInput, public MIRParametrisation {
  public:

    // -- Exceptions
    // None

    // -- Contructors

    GribInput();

    // -- Destructor

    virtual ~GribInput(); // Change to virtual if base class

    // -- Convertors
    // None

    // -- Operators
    // None

    // -- Methods
    // None

    bool next();


    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

  protected:

    // -- Members

    std::auto_ptr<grib_handle> grib_;


    // -- Methods


    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

  private:

    // No copy allowed

    GribInput(const GribInput &);
    GribInput &operator=(const GribInput &);

    // -- Members


    // -- Methods


    // -- Overridden methods
    // From MIRInput

    virtual const MIRParametrisation &parametrisation() const;
    virtual MIRField *field() const;
    virtual grib_handle *gribHandle() const;

    // From MIRParametrisation
    bool get(const std::string &name, std::string &value) const;

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends

    //friend ostream& operator<<(ostream& s,const GribInput& p)
    //  { p.print(s); return s; }

};

#endif
