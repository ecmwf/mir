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


#ifndef mir_netcdf_ValueT_h
#define mir_netcdf_ValueT_h

#include <sstream>
#include <string>
#include <vector>

#include "eckit/utils/Translator.h"

#include "mir/netcdf/Value.h"


namespace mir {
namespace netcdf {


template <class T>
class ValueT : public Value {
public:
    ValueT(Type& type, const T& value) : Value(type), value_(value) {}
    virtual void createAttribute(int nc, int varid, const std::string& name, const std::string& path) override;


private:
    T value_;

    virtual void print(std::ostream& out) const override { out << value_; }

    virtual void dump(std::ostream& out) const override;

    virtual std::string asString() const override {
        std::ostringstream oss;
        oss << *this;
        return oss.str();
    }

    virtual ValueT* clone() const override { return new ValueT(type_, value_); }

    virtual bool sameAs(const Value& other) const override {
        auto o = dynamic_cast<const ValueT*>(&other);
        return (o != nullptr) && (o->value_ == value_);
    }

    template <class U>
    void _fill(const std::vector<bool>& set, std::vector<U>& v) {
        U value = eckit::Translator<T, U>()(value_);

        for (size_t i = 0; i < set.size(); i++) {
            if (!set[i]) {
                v[i] = value;
            }
        }
    }

    virtual void fill(const std::vector<bool>& set, std::vector<double>& v) override { _fill(set, v); }
    virtual void fill(const std::vector<bool>& set, std::vector<short>& v) override { _fill(set, v); }
    virtual void fill(const std::vector<bool>& set, std::vector<unsigned char>& v) override { _fill(set, v); }
    virtual void fill(const std::vector<bool>& set, std::vector<long long>& v) { _fill(set, v); }
    virtual void fill(const std::vector<bool>& set, std::vector<long>& v) override { _fill(set, v); }
    virtual void fill(const std::vector<bool>& set, std::vector<float>& v) override { _fill(set, v); }
    virtual void fill(const std::vector<bool>& set, std::vector<std::string>& v) override { _fill(set, v); }

    template <class U>
    void _clear(std::vector<bool>& set, const std::vector<U>& v) {
        U value = eckit::Translator<T, U>()(value_);

        for (size_t i = 0; i < set.size(); i++) {
            if (v[i] == value) {
                set[i] = false;
            }
        }
    }

    virtual void clear(std::vector<bool>& set, const std::vector<double>& v) override { _clear(set, v); }
    virtual void clear(std::vector<bool>& set, const std::vector<short>& v) override { _clear(set, v); }
    virtual void clear(std::vector<bool>& set, const std::vector<unsigned char>& v) override { _clear(set, v); }
    virtual void clear(std::vector<bool>& set, const std::vector<long long>& v) { _clear(set, v); }
    virtual void clear(std::vector<bool>& set, const std::vector<long>& v) override { _clear(set, v); }
    virtual void clear(std::vector<bool>& set, const std::vector<float>& v) override { _clear(set, v); }
    virtual void clear(std::vector<bool>& set, const std::vector<std::string>& v) override { _clear(set, v); }

    template <class U>
    void _init(std::vector<U>& v, size_t size) {
        U value = eckit::Translator<T, U>()(value_);
        std::vector<U> w(size, value);
        std::swap(v, w);
    }

    virtual void init(std::vector<double>& v, size_t size) override { _init(v, size); }
    virtual void init(std::vector<short>& v, size_t size) override { _init(v, size); }
    virtual void init(std::vector<unsigned char>& v, size_t size) override { _init(v, size); }
    virtual void init(std::vector<long long>& v, size_t size) override { _init(v, size); }
    virtual void init(std::vector<long>& v, size_t size) override { _init(v, size); }
    virtual void init(std::vector<float>& v, size_t size) override { _init(v, size); }
    virtual void init(std::vector<std::string>& v, size_t size) override { _init(v, size); }

    template <class U>
    void _get(U& v) const {
        U value = eckit::Translator<T, U>()(value_);
        v       = value;
    }

    virtual void get(double& v) const override { _get(v); }
    virtual void get(short& v) const override { _get(v); }
    virtual void get(unsigned char& v) const override { _get(v); }
    virtual void get(long long& v) const override { _get(v); }
    virtual void get(long& v) const override { _get(v); }
    virtual void get(float& v) const override { _get(v); }
    virtual void get(std::string& v) const override { _get(v); }
};


}  // namespace netcdf
}  // namespace mir


#endif
