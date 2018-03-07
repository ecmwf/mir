/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Baudouin Raoult
/// @author Pedro Maciel
/// @date Apr 2015


#ifndef mir_output_MIROutput_h
#define mir_output_MIROutput_h

#include <iosfwd>
#include <string>
#include <vector>
#include "eckit/memory/NonCopyable.h"


namespace mir {
namespace context {
class Context;
}
namespace param {
class MIRParametrisation;
}
}


namespace mir {
namespace output {


class MIROutput : private eckit::NonCopyable {
public:

    // -- Exceptions
    // None

    // -- Contructors

    MIROutput();

    // -- Destructor

    virtual ~MIROutput();

    // -- Convertors
    // None

    // -- Operators
    // None

    // -- Methods

    virtual size_t copy(const param::MIRParametrisation&, context::Context&) = 0;
    virtual size_t save(const param::MIRParametrisation&, context::Context&) = 0;
    virtual bool sameAs(const MIROutput& other) const = 0;
    virtual bool sameParametrisation(const param::MIRParametrisation&, const param::MIRParametrisation&) const = 0;
    virtual bool printParametrisation(std::ostream&, const param::MIRParametrisation&) const = 0;

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

    friend std::ostream& operator<<(std::ostream& s, const MIROutput& p) {
        p.print(s);
        return s;
    }

};


class MIROutputFactory {
    const std::string name_;
    const std::vector<std::string>& extensions_;
protected:
    MIROutputFactory(const std::string& name, const std::vector<std::string>& extensions = {});
    virtual ~MIROutputFactory();
    static const std::vector<std::string> no_extensions;
public:
    virtual MIROutput* make(const std::string& path, const param::MIRParametrisation&) = 0;
    static MIROutput* build(const std::string& path, const param::MIRParametrisation&);
    static void list(std::ostream&);
};


template<class T>
class MIROutputBuilder : public MIROutputFactory {
    virtual MIROutput* make(const std::string& path, const param::MIRParametrisation& parametrisation) {
        return new T(path, parametrisation);
    }
public:
    MIROutputBuilder(const std::string& name) : MIROutputFactory(name) {}
    MIROutputBuilder(const std::string& name, const std::vector<std::string>& extensions) :
        MIROutputFactory(name, extensions) {
    }
};


}  // namespace output
}  // namespace mir


#endif

