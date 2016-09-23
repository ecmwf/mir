/*
 * (C) Copyright 1996-2016 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @date Sep 2016


#ifndef mir_param_ParamClass_h
#define mir_param_ParamClass_h

#include <iosfwd>
#include <map>
#include <string>
#include "mir/param/SimpleParametrisation.h"
#include "mir/util/ParserConsumer.h"


namespace mir {
namespace param {


class ParamClass : public util::ParserConsumer {
public:

    // -- Exceptions
    // None

    // -- Contructors
    // None

    // -- Destructor
    // None

    // -- Convertors
    // None

    // -- Operators
    // None

    // -- Methods

    const SimpleParametrisation* lookup(const std::string& paramClass) const;

    bool has(const std::string& paramClass) const;

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods

    static const ParamClass& instance();

protected:
    // -- Contructors

    ParamClass();

    // -- Destructor

    ~ParamClass();

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

    ParamClass(const ParamClass&);
    ParamClass& operator=(const ParamClass&);

    // -- Types

    typedef std::map< std::string, SimpleParametrisation* > map_t;

    // -- Members

    std::string current_;
    SimpleParametrisation* scope_;
    map_t settings_;

    // -- Methods
    // None

    // -- Overridden methods

    // From ParserConsumer
    virtual void store(const std::string& name, const char* value);
    virtual void store(const std::string& name, const std::string& value);
    virtual void store(const std::string& name, bool value);
    virtual void store(const std::string& name, long value);
    virtual void store(const std::string& name, double value);

    virtual void scope(const std::string& name);

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends

    friend std::ostream& operator<<(std::ostream& s, const ParamClass& p) {
        p.print(s);
        return s;
    }

};


}  // namespace param
}  // namespace mir


#endif
