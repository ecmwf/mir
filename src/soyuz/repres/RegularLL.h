// File RegularLL.h
// Baudouin Raoult - (c) ECMWF Apr 15

#ifndef RegularLL_H
#define RegularLL_H

#include "soyuz/repres/Gridded.h"

class RegularLL : public Gridded {
  public:

// -- Exceptions
    // None

// -- Contructors

    RegularLL(const MIRParametrisation&);

// -- Destructor

    virtual ~RegularLL(); // Change to virtual if base class

// -- Convertors
    // None

// -- Operators
    // None

// -- Methods

    size_t ni() const {
        return ni_;
    }
    size_t nj() const {
        return nj_;
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

    void print(std::ostream&) const; // Change to virtual if base class

// -- Overridden methods
    // None

// -- Class members
    // None

// -- Class methods
    // None

  private:

    RegularLL(double north, double west, double south, double east, double north_south_increment, double west_east_increment);

// No copy allowed

    RegularLL(const RegularLL&);
    RegularLL& operator=(const RegularLL&);

// -- Members

    double north_;
    double west_;
    double south_;
    double east_;

    double north_south_increment_;
    double west_east_increment_;

    size_t ni_;
    size_t nj_;

// -- Methods
    // None

    void setNiNj();

// -- Overridden methods

    virtual void fill(grib_info&) const;
    virtual Representation* crop(double north, double west, double south, double east, const std::vector<double>&, std::vector<double>&) const;
    virtual size_t frame(std::vector<double> &values, size_t size, double missingValue) const;

// -- Class members
    // None

// -- Class methods
    // None

// -- Friends

    //friend ostream& operator<<(ostream& s,const RegularLL& p)
    //	{ p.print(s); return s; }

};

#endif
