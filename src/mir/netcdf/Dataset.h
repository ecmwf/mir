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

#include "mir/netcdf/Endowed.h"


namespace mir::netcdf {
class Dimension;
class Variable;
}  // namespace mir::netcdf


namespace mir::netcdf {


class Dataset : public Endowed {
public:
    Dataset(const std::string&);
    ~Dataset() override;

    // -- Methods

    Dimension* findDimension(int id) const;
    Dimension* findDimension(const std::string& name) const;
    std::vector<Variable*> variablesForDimension(const Dimension&) const;

    virtual void dump(std::ostream&, bool data) const;

    void add(Dimension*);
    void add(Variable*);

    const std::map<std::string, Dimension*>& dimensions() const;
    const std::map<std::string, Variable*>& variables() const;

    //
    bool hasVariable(const std::string& name) const;
    const Variable& variable(const std::string& name) const;
    Variable& variable(const std::string& name);

    // From Endowed

    const std::string& path() const override;

protected:
    // -- Members
    std::string path_;
    std::map<std::string, Dimension*> dimensions_;
    std::map<std::string, Variable*> variables_;

private:
    Dataset(const Dataset&);
    Dataset& operator=(const Dataset&);

    // From Endowed

    int varid() const override;
    const std::string& name() const override;

    // - Methods

    virtual void print(std::ostream&) const = 0;

    // -- Friends
    friend std::ostream& operator<<(std::ostream& s, const Dataset& v) {
        v.print(s);
        return s;
    }
};


}  // namespace mir::netcdf
