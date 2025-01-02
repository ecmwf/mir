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


#include <netcdf.h>

#include <sstream>

#include "mir/netcdf/Type.h"

#include <algorithm>
#include <ostream>
#include <vector>

#include "mir/netcdf/Codec.h"
#include "mir/netcdf/Dimension.h"
#include "mir/netcdf/Exceptions.h"
#include "mir/netcdf/Matrix.h"
#include "mir/netcdf/MergePlan.h"
#include "mir/netcdf/Remapping.h"
#include "mir/netcdf/UpdateCoordinateStep.h"
#include "mir/netcdf/ValueT.h"
#include "mir/netcdf/Variable.h"
#include "mir/util/Log.h"


namespace mir::netcdf {


static Type* types_[NC_MAX_ATOMIC_TYPE + 1] = {
    nullptr,
};


Type::Type(int code, const std::string& name, const std::string& dump, int super) :
    code_(code), super_(super), name_(name), dump_(dump) {
    types_[code] = this;
}


Type& Type::lookup(int type) {
    ASSERT(type >= 0 && type <= NC_MAX_ATOMIC_TYPE);

    if (types_[type] == nullptr) {
        Log::error() << "Type::lookup " << type << " is unknown: ";

        switch (type) {

            case NC_BYTE:
                Log::error() << "NC_BYTE" << std::endl;
                break;
            case NC_UBYTE:
                Log::error() << "NC_UBYTE" << std::endl;
                break;
            case NC_CHAR:
                Log::error() << "NC_CHAR" << std::endl;
                break;
            case NC_SHORT:
                Log::error() << "NC_SHORT" << std::endl;
                break;
            case NC_USHORT:
                Log::error() << "NC_USHORT" << std::endl;
                break;
            case NC_INT:
                Log::error() << "NC_INT" << std::endl;
                break;
            case NC_UINT:
                Log::error() << "NC_UINT" << std::endl;
                break;
            case NC_INT64:
                Log::error() << "NC_INT64" << std::endl;
                break;
            case NC_UINT64:
                Log::error() << "NC_UINT64" << std::endl;
                break;
            case NC_FLOAT:
                Log::error() << "NC_FLOAT" << std::endl;
                break;
            case NC_DOUBLE:
                Log::error() << "NC_DOUBLE" << std::endl;
                break;
            case NC_STRING:
                Log::error() << "NC_STRING" << std::endl;
                break;
            default:
                Log::error() << "????" << std::endl;
        }
    }

    ASSERT(types_[type] != nullptr);
    return *types_[type];
}


Type& Type::lookup(Type& type1, Type& type2) {
    if (type1 == type2) {
        return type1;
    }

    Type* t1 = &type1;
    Type* t2 = &type2;

    std::vector<int> s1;
    std::vector<int> s2;

    while (t1->code_ != -1) {
        s1.push_back(t1->code_);
        if (t1->super_ < 0) {
            break;
        }
        t1 = &lookup(t1->super_);
    }

    while (t2->code_ != -1) {
        s2.push_back(t2->code_);
        if (t2->super_ < 0) {
            break;
        }
        t2 = &lookup(t2->super_);
    }

    for (const auto j : s1) {
        auto k = std::find(s2.begin(), s2.end(), j);
        if (k != s2.end()) {

            if (type1 != type2) {
                Log::info() << "Common super-type for " << type1 << " and " << type2 << " is " << lookup(j)
                            << std::endl;
            }

            return lookup(j);
        }
    }

    std::ostringstream s;
    s << "Cannot find a common super-type to " << type1 << " and " << type2;
    throw exception::MergeError(s.str());
}


void Type::dump(std::ostream& out) const {
    out << dump_;
}


bool Type::operator==(const Type& other) const {
    return code_ == other.code_;
}


bool Type::operator!=(const Type& other) const {
    return code_ != other.code_;
}


template <class T>
class TypeT : public Type {

public:
    TypeT(int code, const std::string& name, const std::string& dump, int super) : Type(code, name, dump, super) {}

private:
    Value* attributeValue(int nc, int id, const char* name, size_t len, const std::string& path) override;

    bool coordinateOutputVariableMerge(Variable& out, const Variable& in, MergePlan& plan) override;
    bool cellMethodOutputVariableMerge(Variable& out, const Variable& in, MergePlan& plan) override;
    void save(const Matrix& /*unused*/, int nc, int varid, const std::string& path) const override;

    void print(std::ostream& out) const override;
    void dump(std::ostream& out, const Matrix& /*matrix*/) const override;
    void printValues(std::ostream& out, const Matrix& /*matrix*/) const override;
};

template <class T>
void TypeT<T>::print(std::ostream& out) const {
    out << name_;
}


template <class T>
void TypeT<T>::dump(std::ostream& out, const Matrix& matrix) const {
    const auto& v = matrix.values<T>();
    size_t i      = 0;
    for (auto& k : v) {
        out << ' ' << k;
        i++;
        if (i < v.size()) {
            out << ',';
        }

        if ((i % 7) == 0) {
            out << std::endl;
        }

        if (i > 13) {
            if (i < v.size()) {
                out << " ...";
            }
            break;
        }
    }
}


template <class T>
void TypeT<T>::printValues(std::ostream& out, const Matrix& matrix) const {
    const typename std::vector<T> v = matrix.values<T>();
    size_t i                        = 0;
    for (auto& k : v) {
        out << k;
        i++;
        if (i < v.size()) {
            out << ',';
        }
    }
}


template <>
bool TypeT<std::string>::cellMethodOutputVariableMerge(Variable& /*out*/, const Variable& /*in*/, MergePlan& /*plan*/) {
    std::ostringstream os;
    os << "TypeT<std::string>::cellMethodOutputVariableMerge() not implemented for " << *this;
    throw exception::SeriousBug(os.str());
}


template <class T>
bool TypeT<T>::cellMethodOutputVariableMerge(Variable& out, const Variable& in, MergePlan& /*plan*/) {
    const std::vector<T>& a = out.matrix()->values<T>();
    const std::vector<T>& b = in.matrix()->values<T>();

    return (a != b);
}


template <class T, class Q>
static void save_values(const Matrix& matrix, int nc, int varid, const std::string& path, Q put) {

    // Log::info() << "Save " << matrix << std::endl;
    // matrix.dumpTree(Log::info(), 0);
    Codec* codec = matrix.codec();
    if (codec) {
        std::vector<T> values = matrix.values<T>();
        codec->encode(values);
        ASSERT(varid >= 0);
        ASSERT(values.size());
        NC_CALL(put(nc, varid, values.data()), path);
    }
    else {
        const std::vector<T>& values = matrix.values<T>();
        ASSERT(varid >= 0);
        ASSERT(values.size());
        NC_CALL(put(nc, varid, values.data()), path);
    }
}


template <>
void TypeT<std::string>::save(const Matrix& /*unused*/, int /*nc*/, int /*varid*/, const std::string& /*path*/) const {
    std::ostringstream os;
    os << "TypeT<std::string>::save() not implemented for " << *this;
    throw exception::SeriousBug(os.str());
}


template <>
void TypeT<double>::save(const Matrix& m, int out, int varid, const std::string& path) const {
    save_values<double>(m, out, varid, path, &nc_put_var_double);
}


template <>
void TypeT<float>::save(const Matrix& m, int out, int varid, const std::string& path) const {
    save_values<float>(m, out, varid, path, &nc_put_var_float);
}


template <>
void TypeT<unsigned char>::save(const Matrix& m, int out, int varid, const std::string& path) const {
    save_values<unsigned char>(m, out, varid, path, &nc_put_var_ubyte);
}


template <>
void TypeT<long>::save(const Matrix& m, int out, int varid, const std::string& path) const {
    save_values<long>(m, out, varid, path, &nc_put_var_long);
}


template <>
void TypeT<long long>::save(const Matrix& m, int out, int varid, const std::string& path) const {
    save_values<long long>(m, out, varid, path, &nc_put_var_longlong);
}


template <>
void TypeT<short>::save(const Matrix& m, int out, int varid, const std::string& path) const {
    save_values<short>(m, out, varid, path, &nc_put_var_short);
}


template <>
bool TypeT<std::string>::coordinateOutputVariableMerge(Variable& /*out*/, const Variable& /*in*/, MergePlan& /*plan*/) {
    Log::info() << __func__ << " " << *this << std::endl;
    NOTIMP;
}


template <class T>
bool TypeT<T>::coordinateOutputVariableMerge(Variable& /*out*/, const Variable& /*in*/, MergePlan& /*plan*/) {
#if 0
    const std::vector<T> &a = out.matrix()->values<T>();
    const std::vector<T> &b = in.matrix()->values<T>();

    if (a != b) {

        const std::vector<Dimension *> &dimensions = out.dimensions();
        ASSERT(dimensions.size() == 1);

        size_t growth = 0;
        std::vector<size_t> ignore;
        Remapping *remapping = new Remapping();

        for (size_t i = 0; i < b.size(); i++) {
            bool found = false;
            for (size_t j  = 0; j < a.size(); j++) {
                if (b[i] == a[j]) {
                    remapping->push_back(j);
                    ASSERT(j >= i);
                    found = true;
                    break;
                }
            }
            if (!found) {
                remapping->push_back(a.size() + growth);
                growth++;
            }
        }

        remapping->validate();

        plan.add(new UpdateCoordinateStep(out, in, growth));
        dimensions[0]->remapping(remapping);
        return true;

    }
    else {
        return false;
    }
#else
    // FIXME
    return false;
#endif
}


template <>
Value* TypeT<unsigned char>::attributeValue(int nc, int id, const char* name, size_t len, const std::string& path) {
    unsigned char value;
    ASSERT(len == 1);
    NC_CALL(nc_get_att_ubyte(nc, id, name, &value), path);
    auto* v = new ValueT<unsigned char>(*this, value);
    return v;
}


template <>
Value* TypeT<short>::attributeValue(int nc, int id, const char* name, size_t len, const std::string& path) {
    short value;
    ASSERT(len == 1);
    NC_CALL(nc_get_att_short(nc, id, name, &value), path);
    auto* v = new ValueT<short>(*this, value);
    return v;
}


template <>
Value* TypeT<long>::attributeValue(int nc, int id, const char* name, size_t len, const std::string& path) {
    long value;
    ASSERT(len == 1);
    NC_CALL(nc_get_att_long(nc, id, name, &value), path);
    auto* v = new ValueT<long>(*this, value);
    return v;
}


template <>
Value* TypeT<long long>::attributeValue(int nc, int id, const char* name, size_t len, const std::string& path) {
    long long value;
    ASSERT(len == 1);
    NC_CALL(nc_get_att_longlong(nc, id, name, &value), path);
    auto* v = new ValueT<long long>(*this, value);
    return v;
}


template <>
Value* TypeT<std::string>::attributeValue(int nc, int id, const char* name, size_t len, const std::string& path) {

    if (code_ == NC_STRING) {
        char* value = nullptr;
        NC_CALL(nc_get_att_string(nc, id, name, &value), path);
        ASSERT(value);
        auto* v = new ValueT<std::string>(*this, value);
        return v;
    }

    std::string value(len + 1, '\0');
    NC_CALL(nc_get_att_text(nc, id, name, value.data()), path);
    auto* v = new ValueT<std::string>(*this, value.data());
    return v;
}


template <>
Value* TypeT<double>::attributeValue(int nc, int id, const char* name, size_t len, const std::string& path) {
    double value;
    ASSERT(len == 1);
    NC_CALL(nc_get_att_double(nc, id, name, &value), path);
    auto* v = new ValueT<double>(*this, value);
    return v;
}


template <>
Value* TypeT<float>::attributeValue(int nc, int id, const char* name, size_t len, const std::string& path) {
    float value;
    ASSERT(len == 1);
    NC_CALL(nc_get_att_float(nc, id, name, &value), path);
    auto* v = new ValueT<float>(*this, value);
    return v;
}


#define T(a, b, c) static const TypeT<a> TYPE_##b(b, #b, #a, c)
T(unsigned char, NC_BYTE, NC_SHORT);
T(short, NC_SHORT, NC_LONG);
T(long, NC_LONG, NC_DOUBLE);
T(long long, NC_INT64, -1);
T(std::string, NC_CHAR, -1);
T(float, NC_FLOAT, NC_DOUBLE);
T(double, NC_DOUBLE, -1);
T(std::string, NC_STRING, -1);


}  // namespace mir::netcdf
