/*
 * (C) Copyright 1996-2013 ECMWF.
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
    out << "DataInputVariable[name=" << name_ << ",nc=" << ncname() << ",cube=" << cube() << "]";
}

const std::string &DataInputVariable::ncname() const {
    auto j = attributes_.find("standard_name");
    if (j != attributes_.end()) {
        ncname_ = (*j).second->asString();
        return ncname_;
    }
    return name();
}

void DataInputVariable::collectField(std::vector<Field *>& fields) const {
    fields.push_back(new Field(*this));
}

void DataInputVariable::get2DValues(std::vector<double>& values, size_t i) const {

    size_t size = cube_.dimensions().size();

    ASSERT(size >= 2);


    std::vector<size_t> start(size, 0);
    std::vector<size_t> count(size, 1);

    size_t nx = cube_.dimensions(size - 1);
    size_t ny = cube_.dimensions(size - 2);

    values.resize(nx * ny);

    count[size - 1] = nx;
    count[size - 2] = ny;

    std::cout << "++++++++++++++ " << start << " ------ " << count << std::endl;

    matrix_->read(values, start, count);

}



}
}
