// File MIRLogic.h
// Baudouin Raoult - (c) ECMWF Apr 15

#ifndef MIRLogic_H
#define MIRLogic_H

#include <string>
#include <iosfwd>
#include <vector>
#include <memory>


class MIRParametrisation;
class Action;

class MIRLogic {
  public:

// -- Exceptions
    // None

// -- Contructors

    MIRLogic(const MIRParametrisation& parametrisation);

// -- Destructor

    virtual ~MIRLogic(); // Change to virtual if base class

// -- Convertors
    // None

// -- Operators
    // None

// -- Methods

    virtual void prepare(std::vector<std::auto_ptr<Action> >&) const = 0;

// -- Overridden methods
    // None

// -- Class members
    // None

// -- Class methods
    // None

  protected:

// -- Members

    const MIRParametrisation& parametrisation_;

// -- Methods

    void add(std::vector<std::auto_ptr<Action> >& actions, const std::string& name) const;


    virtual void print(std::ostream&) const = 0; // Change to virtual if base class

// -- Overridden methods
    // None

// -- Class members
    // None

// -- Class methods
    // None

  private:

// No copy allowed

    MIRLogic(const MIRLogic&);
    MIRLogic& operator=(const MIRLogic&);

// -- Members
    // None

// -- Methods
    // None

// -- Overridden methods
    // None

// -- Class members
    // None

// -- Class methods
    // None

// -- Friends

    friend std::ostream& operator<<(std::ostream& s,const MIRLogic& p) {
        p.print(s);
        return s;
    }

};

class MIRLogicFactory {
    std::string name_;
    virtual MIRLogic* make(const MIRParametrisation&) = 0 ;

  protected:

    MIRLogicFactory(const std::string&);
    virtual ~MIRLogicFactory();

  public:
    static MIRLogic* build(const MIRParametrisation&);

};

template<class T>
class MIRLogicBuilder : public MIRLogicFactory {
    virtual MIRLogic* make(const MIRParametrisation& param) {
        return new T(param);
    }
  public:
    MIRLogicBuilder(const std::string& name) : MIRLogicFactory(name) {}
};


#endif
