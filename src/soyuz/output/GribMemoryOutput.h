// File GribMemoryOutput.h
// Baudouin Raoult - (c) ECMWF Apr 15

#ifndef GribMemoryOutput_H
#define GribMemoryOutput_H

// namespace outline;


#include "soyuz/output/GribOutput.h"

class GribMemoryOutput : public GribOutput {
  public:

    // -- Exceptions
    // None

    // -- Contructors

    GribMemoryOutput(void *message, size_t size);

    // -- Destructor

    virtual ~GribMemoryOutput(); // Change to virtual if base class

    // -- Convertors
    // None

    // -- Operators
    // None

    // -- Methods
    // None

    size_t interpolated() const {
        return interpolated_;
    }
    size_t saved() const {
        return saved_;
    }
    size_t length() const {
        return length_;
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


    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

  private:

    // No copy allowed

    GribMemoryOutput(const GribMemoryOutput &);
    GribMemoryOutput &operator=(const GribMemoryOutput &);

    // -- Members

    void* message_;
    size_t size_;
    size_t length_;

    size_t saved_;
    size_t interpolated_;

    // -- Methods

    // -- Overridden methods

    virtual void out(const void* message, size_t length, bool iterpolated);
    virtual void print(std::ostream&) const;

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends

    //friend ostream& operator<<(ostream& s,const GribMemoryOutput& p)
    //  { p.print(s); return s; }

};

#endif
