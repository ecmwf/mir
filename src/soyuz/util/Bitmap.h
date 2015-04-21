// File Bitmap.h
// Baudouin Raoult - (c) ECMWF Apr 15

#ifndef Bitmap_H
#define Bitmap_H


#include "eckit/filesystem/PathName.h"


class Bitmap {
  public:

// -- Exceptions
    // None

// -- Contructors

    Bitmap(const eckit::PathName&);

// -- Destructor

    ~Bitmap(); // Change to virtual if base class

// -- Convertors
    // None

// -- Operators
    // None

// -- Methods

    const size_t width() const {
        return width_;
    }
    const size_t height() const {
        return height_;
    }

    bool on(size_t i, size_t j) const {
        return bitmap_[i][j];
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

// No copy allowed

    Bitmap(const Bitmap&);
    Bitmap& operator=(const Bitmap&);

// -- Members

    eckit::PathName path_;
    std::vector<std::vector<bool> > bitmap_;
    size_t width_;
    size_t height_;

// -- Methods
    // None

// -- Overridden methods


// -- Class members
    // None

// -- Class methods
    // None

// -- Friends

    friend std::ostream& operator<<(std::ostream& s,const Bitmap& p) {
        p.print(s);
        return s;
    }

};

#endif
