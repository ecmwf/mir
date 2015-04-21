// File MIRCombinedParametrisation.h
// Baudouin Raoult - (c) ECMWF Apr 15

#ifndef MIRCombinedParametrisation_H
#define MIRCombinedParametrisation_H

#include "soyuz/param/MIRParametrisation.h"

class MIRCombinedParametrisation : public MIRParametrisation {
  public:

// -- Exceptions
    // None

// -- Contructors

    MIRCombinedParametrisation(const MIRParametrisation& user,
                               const MIRParametrisation& metadata,
                               const MIRParametrisation& configuration,
                               const MIRParametrisation& defaults);

// -- Destructor

    ~MIRCombinedParametrisation(); // Change to virtual if base class

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

    // void print(ostream&) const; // Change to virtual if base class

// -- Overridden methods
    // None

// -- Class members
    // None

// -- Class methods
    // None

  private:

// No copy allowed

    MIRCombinedParametrisation(const MIRCombinedParametrisation&);
    MIRCombinedParametrisation& operator=(const MIRCombinedParametrisation&);

// -- Members
    const MIRParametrisation& user_;
    const MIRParametrisation& metadata_;
    const MIRParametrisation& configuration_;
    const MIRParametrisation& defaults_;

// -- Methods
    // None

    template<class T>
    bool _get(const std::string&, T&) const;

// -- Overridden methods

    // From MIRParametrisation
    virtual void print(std::ostream&) const;
    virtual bool get(const std::string&, std::string&) const;

// -- Class members
    // None

// -- Class methods
    // None

// -- Friends

    //friend ostream& operator<<(ostream& s,const MIRCombinedParametrisation& p)
    //	{ p.print(s); return s; }

};

#endif
