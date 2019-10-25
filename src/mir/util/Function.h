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
/// @author Tiago Quintino
/// @author Pedro Maciel
/// @date April 2016


#ifndef mir_util_Function_h
#define mir_util_Function_h

#include <string>
#include <vector>

#include "mir/util/Formula.h"


namespace mir {
namespace util {


class Function {
public:

    Function(const std::string& name);

    Function(const Function&) = delete;
    void operator=(const Function&) = delete;

    virtual ~Function();

    virtual void execute(context::Context& ctx) const = 0;


    static const Function& lookup(const std::string& name);

private:

    virtual void print(std::ostream& s) const = 0;

protected:

    const std::string name_;

private:

    friend std::ostream &operator<<(std::ostream &s, const Function &p) {
        p.print(s);
        return s;
    }

};


}  // namespace util
}  // namespace mir


#endif

