/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 *
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#ifndef mir_input_MIRInput_h
#define mir_input_MIRInput_h

#include <iosfwd>
#include <string>


// Forward declaration only, not need for grib_api
using grib_handle = struct grib_handle;


namespace mir {
namespace data {
class MIRField;
}
namespace param {
class MIRParametrisation;
}
}  // namespace mir


namespace mir {
namespace input {


class MIRInput {

protected:
    // -- Constructors

    MIRInput();

public:
    // -- Constructors

    MIRInput(const MIRInput&) = delete;

    // -- Destructor

    virtual ~MIRInput();

    // -- Convertors
    // None

    // -- Operators

    void operator=(const MIRInput&) = delete;

    // -- Methods

    virtual bool next();
    virtual size_t dimensions() const;
    virtual const param::MIRParametrisation& parametrisation(size_t which = 0) const = 0;
    virtual data::MIRField field() const = 0;
    virtual grib_handle* gribHandle(size_t which = 0) const;
    virtual void setAuxiliaryInformation(const std::string&);
    virtual size_t copy(double* values, size_t size) const;
    virtual bool sameAs(const MIRInput& other) const = 0;

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

    virtual void print(std::ostream&) const = 0;

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

private:
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

    friend std::ostream& operator<<(std::ostream& s, const MIRInput& p) {
        p.print(s);
        return s;
    }
};


class MIRInputFactory {

    void operator=(const MIRInputFactory&)  = delete;
    MIRInputFactory(const MIRInputFactory&) = delete;

    unsigned long magic_;

    virtual MIRInput* make(const std::string& path) = 0;

protected:
    MIRInputFactory(unsigned long magic);
    virtual ~MIRInputFactory();

public:
    static MIRInput* build(const std::string&, const param::MIRParametrisation&);
    static void list(std::ostream&);
};


template <class T>
class MIRInputBuilder : public MIRInputFactory {
    virtual MIRInput* make(const std::string& path) { return new T(path); }

public:
    MIRInputBuilder(unsigned long magic) : MIRInputFactory(magic) {}
};


}  // namespace input
}  // namespace mir


#endif
