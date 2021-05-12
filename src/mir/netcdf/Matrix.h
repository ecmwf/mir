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

#include "eckit/memory/Counted.h"


namespace mir {
namespace netcdf {
class Type;
class Value;
class Codec;
}  // namespace netcdf
}  // namespace mir


namespace mir {
namespace netcdf {


class Matrix : public eckit::Counted {
public:
    Matrix(Type& type, const std::string& name, size_t size);

    Type& type() const;
    void type(Type&);

    const std::string& name() const;
    size_t size() const;

    virtual void read(std::vector<double>&) const;
    virtual void read(std::vector<float>&) const;
    virtual void read(std::vector<long>&) const;
    virtual void read(std::vector<short>&) const;
    virtual void read(std::vector<unsigned char>&) const;
    virtual void read(std::vector<long long>&) const;
    virtual void read(std::vector<std::string>&) const;

    virtual void read(std::vector<double>&, const std::vector<size_t>& start, const std::vector<size_t>& count) const;
    virtual void read(std::vector<float>&, const std::vector<size_t>& start, const std::vector<size_t>& count) const;
    virtual void read(std::vector<long>&, const std::vector<size_t>& start, const std::vector<size_t>& count) const;
    virtual void read(std::vector<short>&, const std::vector<size_t>& start, const std::vector<size_t>& count) const;
    virtual void read(std::vector<unsigned char>&, const std::vector<size_t>& start,
                      const std::vector<size_t>& count) const;
    virtual void read(std::vector<long long>&, const std::vector<size_t>& start,
                      const std::vector<size_t>& count) const;

    template <class T>
    std::vector<T> values() const {
        std::vector<T> result;
        read(result);
        return result;
    }

public:
    void dump(std::ostream&) const;
    virtual void dumpTree(std::ostream&, size_t) const;
    void save(int nc, int varid, const std::string& path) const;

    void printValues(std::ostream&) const;
    void missingValue(Value*);
    Value* missingValue() const;

    void codec(Codec*);
    Codec* codec() const;

protected:
    ~Matrix() override;

    // -- Members

    Type* type_;
    Value* missingValue_;
    Codec* codec_;
    std::string name_;
    size_t size_;

private:
    Matrix(const Matrix&);
    Matrix& operator=(const Matrix&);

    // -- Methods

    virtual void print(std::ostream&) const = 0;

    // -- Friends
    friend std::ostream& operator<<(std::ostream& out, const Matrix& v) {
        v.print(out);
        return out;
    }
};


}  // namespace netcdf
}  // namespace mir
