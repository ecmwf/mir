// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


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
