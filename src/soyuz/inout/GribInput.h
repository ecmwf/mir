// File GribInput.h
// Baudouin Raoult - (c) ECMWF Apr 15

#ifndef GribInput_H
#define GribInput_H

// namespace outline;


#include "soyuz/inout/MIRInput.h"
#include "soyuz/param/FieldParametrisation.h"
#include <memory>
#include <map>
#include <string>

class GribInput : public MIRInput, public FieldParametrisation {
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



    // -- Methods

    bool handle(grib_handle*);


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

    std::auto_ptr<grib_handle> grib_;
    mutable std::map<std::string, std::string> cache_;

    // -- Methods


    // -- Overridden methods
    // From MIRInput

    virtual const MIRParametrisation &parametrisation() const;
    virtual MIRField *field() const;
    virtual grib_handle *gribHandle() const;

    // From FieldParametrisation
    bool lowLevelGet(const std::string &name, std::string &value) const;

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends

    //friend ostream& operator<<(ostream& s,const GribInput& p)
    //  { p.print(s); return s; }

};

#endif
