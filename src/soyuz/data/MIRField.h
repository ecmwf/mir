// File MIRField.h
// Baudouin Raoult - (c) ECMWF Apr 15

#ifndef MIRField_H
#define MIRField_H

#include <iosfwd>
#include <vector>

class Representation;

class MIRField {
  public:

// -- Exceptions
    // None

// -- Contructors

    //
    MIRField(bool hasMissing, double missingValue);

// -- Destructor

    ~MIRField(); // Change to virtual if base class

// -- Convertors
    // None

// -- Operators
    // None

// -- Methods

    const std::vector<double>& values() const {
        return values_;
    }

    // Non-const version for direct update (Filter)
    std::vector<double>& values()  {
        return values_;
    }


    bool hasMissing() const {
        return hasMissing_;
    }
    double missingValue() const {
        return missingValue_;
    }


    // Warning Takes ownership of the vector
    void values(std::vector<double>&);
    void hasMissing(bool on) {
        hasMissing_ = on;
    }
    void missingValue(double value)  {
        missingValue_ = value;
    }


    const Representation* representation() const;
    void representation(Representation*);

    //

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


// -- Members

    std::vector<double> values_;
    bool hasMissing_;
    double missingValue_;
    Representation* representation_;

// -- Methods
    // None

// -- Overridden methods
    // None

// -- Class members
    // None

// -- Class methods
    // None

// -- Friends

    friend std::ostream& operator<<(std::ostream& s,const MIRField& p) {
        p.print(s);
        return s;
    }

};

#endif
