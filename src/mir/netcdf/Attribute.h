// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include <iosfwd>
#include <string>


namespace mir::netcdf {
class Endowed;
class Value;
}  // namespace mir::netcdf


namespace mir::netcdf {


class Attribute {
public:
    Attribute(Endowed& owner, const std::string& name, Value* value);
    virtual ~Attribute();

    // -- Methods
    virtual bool sameAs(const Attribute&) const;
    virtual void dump(std::ostream&) const;
    virtual void create(int nc) const;
    virtual void clone(Endowed& owner) const;
    virtual void merge(const Attribute&);
    virtual void invalidate();

    virtual std::string asString() const;

    const std::string& name() const;
    std::string fullName() const;

    const Value& value() const;

protected:
    // -- Members

    Endowed& owner_;
    std::string name_;
    Value* value_;

private:
    Attribute(const Attribute&);
    Attribute& operator=(const Attribute&);

    // -- Methods

    virtual void print(std::ostream&) const = 0;

    // Friends
    friend std::ostream& operator<<(std::ostream& out, const Attribute& v) {
        v.print(out);
        return out;
    }
};


}  // namespace mir::netcdf
