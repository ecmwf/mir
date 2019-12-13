/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#ifndef mir_input_ArtificialInput_h
#define mir_input_ArtificialInput_h

#include "mir/data/MIRField.h"
#include "mir/input/MIRInput.h"
#include "mir/param/FieldParametrisation.h"
#include "mir/param/SimpleParametrisation.h"


namespace mir {
namespace input {


class ArtificialInput : public MIRInput, protected param::FieldParametrisation {
public:
    // -- Destructor

    virtual ~ArtificialInput();

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
    // -- Constructors

    ArtificialInput(const param::MIRParametrisation&);

    // -- Members
    // None

    // -- Methods

    virtual data::MIRValuesVector fill(size_t) const = 0;
    param::SimpleParametrisation& parametrisation(size_t which = 0);

    // -- Overridden methods

    // From MIRInput
    virtual bool next();
    virtual size_t dimensions() const;
    virtual const param::MIRParametrisation& parametrisation(size_t which = 0) const;
    virtual data::MIRField field() const;
    virtual void setAuxiliaryInformation(const std::string& yaml);
    virtual void print(std::ostream&) const;
    virtual bool sameAs(const MIRInput&) const;

    // From FieldParametrisation
    virtual bool has(const std::string& name) const;

    virtual bool get(const std::string& name, std::string& value) const;
    virtual bool get(const std::string& name, bool& value) const;
    virtual bool get(const std::string& name, int& value) const;
    virtual bool get(const std::string& name, long& value) const;
    virtual bool get(const std::string& name, float& value) const;
    virtual bool get(const std::string& name, double& value) const;

    virtual bool get(const std::string& name, std::vector<int>& value) const;
    virtual bool get(const std::string& name, std::vector<long>& value) const;
    virtual bool get(const std::string& name, std::vector<float>& value) const;
    virtual bool get(const std::string& name, std::vector<double>& value) const;
    virtual bool get(const std::string& name, std::vector<std::string>& value) const;

    // -- Class members
    // None

    // -- Class methods
    // None

private:
    // -- Members

    param::SimpleParametrisation parametrisation_;
    size_t calls_;

    // -- Methods
    // None

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends

    friend std::ostream& operator<<(std::ostream& s, const ArtificialInput& p) {
        p.print(s);
        return s;
    }
};


class ArtificialInputFactory {
    ArtificialInputFactory(const ArtificialInputFactory&) = delete;
    void operator=(const ArtificialInputFactory&)                   = delete;
    virtual ArtificialInput* make(const param::MIRParametrisation&) = 0;
    const std::string name_;

protected:
    ArtificialInputFactory(const std::string& name);
    virtual ~ArtificialInputFactory();

public:
    static ArtificialInput* build(const std::string&, const param::MIRParametrisation&);
    static void list(std::ostream&);
};


template <class T>
class ArtificialInputBuilder : public ArtificialInputFactory {
    virtual ArtificialInput* make(const param::MIRParametrisation& param) { return new T(param); }

public:
    ArtificialInputBuilder(const std::string& name) : ArtificialInputFactory(name) {}
};


}  // namespace input
}  // namespace mir


#endif
