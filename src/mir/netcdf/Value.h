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

#include <string>
#include <vector>


namespace mir {
namespace netcdf {
class Type;
}
}  // namespace mir


namespace mir {
namespace netcdf {


class Value {
public:
    Value(Type& type);
    virtual ~Value();

    virtual void createAttribute(int nc, int varid, const std::string& name, const std::string& path) = 0;
    virtual Value* clone() const                                                                      = 0;
    virtual void dump(std::ostream&) const                                                            = 0;

    virtual bool sameAs(const Value&) const = 0;
    virtual std::string asString() const    = 0;

    virtual void fill(const std::vector<bool>& set, std::vector<double>& v)        = 0;
    virtual void fill(const std::vector<bool>& set, std::vector<short>& v)         = 0;
    virtual void fill(const std::vector<bool>& set, std::vector<unsigned char>& v) = 0;
    virtual void fill(const std::vector<bool>& set, std::vector<long long>& v)     = 0;
    virtual void fill(const std::vector<bool>& set, std::vector<long>& v)          = 0;
    virtual void fill(const std::vector<bool>& set, std::vector<float>& v)         = 0;
    virtual void fill(const std::vector<bool>& set, std::vector<std::string>& v)   = 0;

    virtual void clear(std::vector<bool>& set, const std::vector<double>& v)        = 0;
    virtual void clear(std::vector<bool>& set, const std::vector<short>& v)         = 0;
    virtual void clear(std::vector<bool>& set, const std::vector<unsigned char>& v) = 0;
    virtual void clear(std::vector<bool>& set, const std::vector<long long>& v)     = 0;
    virtual void clear(std::vector<bool>& set, const std::vector<long>& v)          = 0;
    virtual void clear(std::vector<bool>& set, const std::vector<float>& v)         = 0;
    virtual void clear(std::vector<bool>& set, const std::vector<std::string>& v)   = 0;

    virtual void init(std::vector<double>& v, size_t size)        = 0;
    virtual void init(std::vector<short>& v, size_t size)         = 0;
    virtual void init(std::vector<unsigned char>& v, size_t size) = 0;
    virtual void init(std::vector<long long>& v, size_t size)     = 0;
    virtual void init(std::vector<long>& v, size_t size)          = 0;
    virtual void init(std::vector<float>& v, size_t size)         = 0;
    virtual void init(std::vector<std::string>& v, size_t size)   = 0;

    virtual void get(double& v) const        = 0;
    virtual void get(short& v) const         = 0;
    virtual void get(unsigned char& v) const = 0;
    virtual void get(long long& v) const     = 0;
    virtual void get(long& v) const          = 0;
    virtual void get(float& v) const         = 0;
    virtual void get(std::string& v) const   = 0;

    static Value* newFromString(const std::string&);

protected:
    // -- Members
    Type& type_;

private:
    Value(const Value&);
    Value& operator=(const Value&);

    // -- Methods
    virtual void print(std::ostream& out) const = 0;

    // -- Friends
    friend std::ostream& operator<<(std::ostream& out, const Value& v) {
        v.print(out);
        return out;
    }
};


}  // namespace netcdf
}  // namespace mir
