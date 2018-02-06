/*
 * (C) Copyright 1996-2017 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#ifndef mir_param_Rules_h
#define mir_param_Rules_h

#include <map>


namespace mir {
namespace param {

class MIRParametrisation;
class SimpleParametrisation;


class Rules  {
protected:

    // -- Types

public:

    // -- Contructors

    Rules();

    // -- Destructor

    virtual ~Rules();

    // -- Operators

    const MIRParametrisation& lookup(const std::string& ruleName, long ruleValue);

    // -- Methods

    void readConfigurationFiles();

private:

    // -- Rules

    std::map<long, SimpleParametrisation*> rules_;

    // -- Methods

    SimpleParametrisation& lookup(long paramId);
    void load(const std::string& kind, const std::string& path);


    virtual void print(std::ostream&) const;

    // -- Friends

    friend std::ostream& operator<<(std::ostream& s, const Rules& p) {
        p.print(s);
        return s;
    }

};


}  // namespace param
}  // namespace mir


#endif

