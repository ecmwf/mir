/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

// Baudouin Raoult - ECMWF Jan 2015

#include "eckit/types/Types.h"

#include "mir/netcdf/DataInputVariable.h"
#include "mir/netcdf/Attribute.h"
#include "mir/netcdf/DataOutputVariable.h"
#include "mir/netcdf/Field.h"
#include "mir/netcdf/Matrix.h"
#include "mir/netcdf/Dimension.h"
#include "mir/netcdf/Dataset.h"

namespace mir {
namespace netcdf {

DataInputVariable::DataInputVariable(Dataset &owner,
                                     const std::string &name,
                                     int id,
                                     const std::vector<Dimension *> &dimensions):
    InputVariable(owner, name, id, dimensions)
{
}

DataInputVariable::~DataInputVariable() {
}

Variable *DataInputVariable::makeOutputVariable(Dataset &owner,
        const std::string &name,
        const std::vector<Dimension *> &dimensions) const {
    return new DataOutputVariable(owner, name, dimensions);
}

void DataInputVariable::print(std::ostream &out) const {
    out << "DataInputVariable[name=" << name_ << ",nc=" << ncname() << "]";
}

const std::string &DataInputVariable::ncname() const {
    auto j = attributes_.find("standard_name");
    if (j != attributes_.end()) {
        ncname_ = (*j).second->asString();
        return ncname_;
    }
    return name();
}


void DataInputVariable::addCoordinateVariable(const Variable* v) {
    for (auto j = coordinates_.begin(); j != coordinates_.end(); ++j) {
        if ((*j) == v) {
            return;
        }
    }
    coordinates_.push_back(v);
}

Variable* DataInputVariable::addMissingCoordinates() {
    for (auto d : dimensions_) {
        bool found = false;
        for (auto c = coordinates_.begin(); !found && c != coordinates_.end(); ++c) {
            found = (d->name() == (*c)->name());
        }
        if (!found) {
            eckit::Log::warning() << "Variable '"
                                  << name()
                                  << ": dimension '"
                                  << d->name()
                                  << "' not listed in coordinates"
                                  << std::endl;

            if (dataset_.hasVariable(d->name())) {
                addCoordinateVariable(&dataset_.variable(d->name()));
            }

        }
    }
    return this;
}


void DataInputVariable::collectField(std::vector<Field *>& fields) const {
    fields.push_back(new Field(*this));
}

size_t DataInputVariable::count2DValues() const {

    std::vector<size_t> dims;
    for (auto d : dimensions_) {
        d->realDimensions(dims);
    }

    // HyperCube::Dimensions dims = cube_.dimensions();
    ASSERT(dims.size() >= 2);
    dims.pop_back();
    dims.pop_back();


    return std::accumulate(dims.begin(), dims.end(), 1, std::multiplies<size_t>());

}

void DataInputVariable::get2DValues(std::vector<double>& values, size_t index) const {

    std::vector<size_t> dims;
    for (auto d : dimensions_) {
        d->realDimensions(dims);
    }

    size_t size = dims.size();

    std::vector<size_t> start(size, 0);
    std::vector<size_t> count(size, 1);

    size_t nx = dims[size - 1];
    size_t ny = dims[size - 2];

    values.resize(nx * ny);

    count[size - 1] = nx;
    count[size - 2] = ny;

    // Get cube from other dimensions
    dims.pop_back();
    dims.pop_back();

    std::vector<size_t>  coords(dims.size());
    for (int i = dims.size() - 1; i >= 0; i--)
    {
        coords[i] = (index % dims[i]);
        index    /= dims[i];
    }

    for (size_t j = 0; j < coords.size(); ++j) {
        start[j] = coords[j];
    }

    matrix_->read(values, start, count);

}


const char* DataInputVariable::kind() const {
    return "data";
}



void DataInputVariable::dumpAttributes(std::ostream &s, const char* prefix) const {
    s << prefix << "Coordinates:" << std::endl;
    for (auto c : coordinates_) {
        s << prefix << "    " << *c << std::endl;
    }
}


std::vector<std::string> DataInputVariable::coordinates() const {
    std::vector<std::string> result;
    for (auto c : coordinates_) {
        result.push_back(c->name());
    }
    return result;
}

}
}
