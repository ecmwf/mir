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


namespace mir::netcdf {
class Value;
class Matrix;
class Variable;
class MergePlan;
}  // namespace mir::netcdf


namespace mir::netcdf {


class Type {
public:
    // -- Methods

    int code() const { return code_; }

    bool operator==(const Type&) const;
    bool operator!=(const Type&) const;

    virtual Value* attributeValue(int nc, int id, const char* name, size_t len, const std::string& path) = 0;
    virtual void save(const Matrix&, int nc, int varid, const std::string& path) const                   = 0;

    virtual bool coordinateOutputVariableMerge(Variable& a, const Variable& b, MergePlan&) = 0;
    virtual bool cellMethodOutputVariableMerge(Variable& a, const Variable& b, MergePlan&) = 0;

    virtual void dump(std::ostream&) const;
    virtual void dump(std::ostream&, const Matrix&) const        = 0;
    virtual void printValues(std::ostream&, const Matrix&) const = 0;


    // -- Class methods
    //
    static Type& lookup(int type);
    static Type& lookup(Type& type1, Type& type2);

protected:
    Type(int code, const std::string& name, const std::string& dump, int super);
    virtual ~Type() = default;

    // -- Members
    int code_;
    int super_;

    std::string name_;
    std::string dump_;


private:
    Type(const Type&);
    Type& operator=(const Type&);

    // -- Methods
    virtual void print(std::ostream&) const = 0;

    // -- Friends
    friend std::ostream& operator<<(std::ostream& s, const Type& v) {
        v.print(s);
        return s;
    }
};


}  // namespace mir::netcdf
