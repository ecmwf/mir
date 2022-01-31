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


#include "mir/netcdf/Dataset.h"

#include "mir/netcdf/Attribute.h"
#include "mir/netcdf/Dimension.h"
#include "mir/netcdf/Exceptions.h"
#include "mir/netcdf/Variable.h"

#include <netcdf.h>

namespace mir {
namespace netcdf {


Dataset::Dataset(const std::string& path) : path_(path) {}


Dataset::~Dataset() {
    for (auto& j : dimensions_) {
        delete j.second;
    }

    for (auto& j : variables_) {
        delete j.second;
    }
}


void Dataset::add(Dimension* d) {
    dimensions_[d->name()] = d;
}


void Dataset::add(Variable* v) {
    // Note: this is 'ncname'
    variables_[v->ncname()] = v;
}


const std::map<std::string, Dimension*>& Dataset::dimensions() const {
    return dimensions_;
}


const std::map<std::string, Variable*>& Dataset::variables() const {
    return variables_;
}


const std::string& Dataset::path() const {
    return path_;
}


const std::string& Dataset::name() const {
    static const std::string empty;
    return empty;
}


int Dataset::varid() const {
    return NC_GLOBAL;
}


Dimension* Dataset::findDimension(int id) const {
    for (const auto& j : dimensions_) {
        if ((j.second)->id() == id) {
            return (j.second);
        }
    }
    ASSERT(false);
    return nullptr;
}


Dimension* Dataset::findDimension(const std::string& name) const {
    for (const auto& j : dimensions_) {
        if ((j.second)->name() == name) {
            return (j.second);
        }
    }
    ASSERT(false);
    return nullptr;
}


void Dataset::dump(std::ostream& out, bool data) const {

    out << "netcdf " << path_ << "{" << std::endl;
    out << "dimensions:" << std::endl;
    for (const auto& j : dimensions_) {
        (j.second)->dump(out);
    }
    out << "variables:" << std::endl;
    for (const auto& j : variables_) {
        (j.second)->dump(out);
    }
    out << "// global attributes:" << std::endl;
    for (const auto& j : attributes_) {
        (j.second)->dump(out);
    }
    if (data) {
        out << std::endl << "data:" << std::endl;
        for (const auto& j : variables_) {
            out << std::endl;
            (j.second)->dumpData(out);
        }
    }

    out << std::endl << "}" << std::endl;
}


std::vector<Variable*> Dataset::variablesForDimension(const Dimension& dim) const {
    std::vector<Variable*> result;
    for (const auto& j : variables_) {
        std::vector<Dimension*> dimensions = (j.second)->dimensions();
        for (auto& k : dimensions) {
            if (k == &dim) {
                result.push_back(j.second);
                break;
            }
        }
    }
    return result;
}


const Variable& Dataset::variable(const std::string& name) const {
    auto j = variables_.find(name);
    if (j == variables_.end()) {
        throw exception::UserError("Cannot find netcdf variable '" + name + "'");
    }
    return *(j->second);
}


Variable& Dataset::variable(const std::string& name) {
    auto j = variables_.find(name);
    if (j == variables_.end()) {
        throw exception::UserError("Cannot find netcdf variable '" + name + "'");
    }
    return *(j->second);
}


bool Dataset::hasVariable(const std::string& name) const {
    return (variables_.find(name) != variables_.end());
}


}  // namespace netcdf
}  // namespace mir
