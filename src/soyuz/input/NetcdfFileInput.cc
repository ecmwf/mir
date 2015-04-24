/*
 * (C) Copyright 1996-2015 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Baudouin Raoult
/// @author Pedro Maciel
/// @date Apr 2015


#include "eckit/exception/Exceptions.h"
#include "eckit/utils/Translator.h"

#include "soyuz/data/MIRField.h"

#include "soyuz/input/NetcdfFileInput.h"

// JUST A DEMO !!!!
// Assumes that netcdf is single 2D variable, that dimensions are called "latitude" and "longitude"
// ... and many more assumptions.

#include <netcdf.h>


namespace mir {
namespace input {
namespace {


inline int _nc_call(int e, const char *call, const std::string &path) {
    if (e) {
        eckit::StrStream os;
        os << call << ": " << nc_strerror(e) << " (" << path << ")" << eckit::StrStream::ends;
        throw eckit::SeriousBug(std::string(os));
    }
    return e;
}


#define NC_CALL(a, path) _nc_call(a, #a, path)


}  // (anonymous namespace)


NetcdfFileInput::NetcdfFileInput(const eckit::PathName &path, const std::string &variable):
    path_(path),
    variable_(variable),
    nc_(-1) {
}


NetcdfFileInput::~NetcdfFileInput() {
    if (nc_ != -1) {
        NC_CALL(nc_close(nc_), path_);
    }
}


void NetcdfFileInput::print(std::ostream &out) const {
    out << "NetcdfFileInput[path=" << path_ << ",variable=" << variable_ << "]";
}


const param::MIRParametrisation &NetcdfFileInput::parametrisation() const {
    return *this;
}


void NetcdfFileInput::getVariable(const std::string &variable, std::vector<double> &values) const {
    if (nc_ == -1) {
        NC_CALL(nc_open(path_.asString().c_str(), NC_NOWRITE, &nc_), path_);
    }

    int varid = -1;

    char name[NC_MAX_NAME + 1];

    int number_of_dimensions;
    int number_of_variables;
    int number_of_global_attributes;
    int id_of_unlimited_dimension;

    NC_CALL(nc_inq(nc_, &number_of_dimensions, &number_of_variables,
                   &number_of_global_attributes,
                   &id_of_unlimited_dimension), path_);


    for (size_t i = 0; i < number_of_dimensions; i++) {
        size_t count;
        NC_CALL(nc_inq_dim(nc_, i, name, &count), path_);
    }

    for (int i = 0; i < number_of_variables; i++) {
        int type;
        int ndims, nattr;
        int dims[NC_MAX_VAR_DIMS];

        NC_CALL(nc_inq_var(nc_, i, name, &type, &ndims, dims, &nattr), path_);

        if (variable == name) {
            varid = i;

            size_t size = 1;
            for (int d = 0; d < ndims; d++ ) {
                size_t count;
                NC_CALL(nc_inq_dim(nc_, dims[d], name, &count), path_);
                size *= count;
            }

            std::vector<double> v(size);

            NC_CALL(nc_get_var_double( nc_, varid, &v[0]), path_);

            std::swap(v, values);
            return;

        }

        // Type &kind = Type::lookup(type);

        // std::vector<Dimension *> dimensions;
        // for (size_t j = 0; j < ndims; j++) {
        //     dimensions.push_back(findDimension(dims[j]));
        // }

        // Variable *v = new SimpleInputVariable(*this, name, i, dimensions);
        // v->setMatrix(new InputMatrix(kind, i, name, v->numberOfValues(), file));
        // v->getAttributes(nc, i, nattr);
        // add(v);
    }

    eckit::StrStream os;
    os <<  "NetcdfFileInput: cannot find variable " << variable << eckit::StrStream::ends;
    throw eckit::SeriousBug(std::string(os));
}


data::MIRField *NetcdfFileInput::field() const {

    std::vector<double> values;
    getVariable(variable_, values);
    std::cout << "Values " << values.size() << std::endl;

    bool hasMissing = false; // Should check!
    double missingValue = 9999; // Read from file

    data::MIRField *field = new data::MIRField(hasMissing, missingValue);
    field->values(values);
    return field;

}


bool NetcdfFileInput::lowLevelGet(const std::string &name, std::string &value) const {

    if (name == "gridType") {
        value = "regular_ll";
        return true;
    }

    eckit::Translator<double, std::string> d2s;

    // Extremly inefficient code:

    if (latitude_.size() == 0) {
        getVariable("latitude", latitude_);
        ASSERT(latitude_.size() >= 2);
        ASSERT(latitude_[0] > latitude_[1]);
    }

    if (longitude_.size() == 0) {
        getVariable("longitude", longitude_);
        ASSERT(longitude_.size() >= 2);
        ASSERT(longitude_[0] < longitude_[1]);
    }

    if (name == "north") {
        value = d2s(latitude_[0]);
        return true;
    }

    if (name == "south") {
        value = d2s(latitude_[latitude_.size() - 1]);
        return true;
    }

    if (name == "west") {
        value = d2s(longitude_[0]);
        return true;
    }

    if (name == "east") {
        value = d2s(longitude_[longitude_.size() - 1]);
        return true;
    }

    if (name == "west_east_increment") {
        value = d2s(longitude_[1] - longitude_[0]);
        return true;
    }

    if (name == "north_south_increment") {
        value = d2s(latitude_[0] - latitude_[1]);
        return true;
    }


    return false;
}


#undef NC_CALL


}  // namespace input
}  // namespace mir

