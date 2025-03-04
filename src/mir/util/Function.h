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


#pragma once

#include <iosfwd>
#include <string>


namespace mir::context {
class Context;
}  // namespace mir::context


namespace mir::util {


class Function {
public:
    Function(const std::string&);

    Function(const Function&) = delete;
    Function(Function&&)      = delete;

    void operator=(const Function&) = delete;
    void operator=(Function&&)      = delete;

    virtual ~Function();

    virtual void execute(context::Context&) const = 0;

    static const Function& lookup(const std::string&);

    static void list(std::ostream&);

private:
    virtual void print(std::ostream&) const = 0;

protected:
    const std::string name_;

private:
    friend std::ostream& operator<<(std::ostream& s, const Function& p) {
        p.print(s);
        return s;
    }
};


}  // namespace mir::util
