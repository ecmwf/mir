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


#include "mir/netcdf/Field.h"

#include <ostream>

#include "eckit/parser/YAMLParser.h"

#include "mir/config/LibMir.h"
#include "mir/data/MIRField.h"
#include "mir/netcdf/GridSpec.h"
#include "mir/netcdf/Variable.h"
#include "mir/util/Log.h"
#include "mir/util/Mutex.h"


namespace mir {
namespace netcdf {


Field::Field(const Variable& variable) :
    variable_(variable),
    standardName_(variable.getAttributeValue<std::string>("standard_name")),
    units_(variable.getAttributeValue<std::string>("units")) {}


Field::~Field() = default;


const GridSpec& Field::gridSpec() const {
    if (!gridSpec_) {
        // TODO: may need a mutex
        gridSpec_.reset(GridSpec::create(variable_));
        Log::info() << *gridSpec_ << std::endl;
    }
    return *gridSpec_;
}


void Field::get2DValues(MIRValuesVector& values, size_t i) const {
    variable_.get2DValues(values, i);
    gridSpec().reorder(values);
}


size_t Field::count2DValues() const {
    return variable_.count2DValues();
}


bool Field::hasMissing() const {
    return variable_.hasMissing();
}


double Field::missingValue() const {
    return variable_.missingValue();
}


bool Field::has(const std::string& name) const {
    return gridSpec().has(name);
}


bool Field::get(const std::string& name, long& value) const {
    constexpr long MISSING = 255;

    if (name == "paramId") {
        value = MISSING;
        return true;
    }
    return gridSpec().get(name, value);
}


bool Field::get(const std::string& name, std::string& value) const {
    return gridSpec().get(name, value);
}


bool Field::get(const std::string& name, double& value) const {
    return gridSpec().get(name, value);
}


bool Field::get(const std::string& name, std::vector<double>& value) const {
    return gridSpec().get(name, value);
}


void Field::print(std::ostream& out) const {
    out << "Field[variable=" << variable_ << "]";
}


static util::once_flag once;
static eckit::Value standard_names;


static void init() {
    standard_names = eckit::YAMLParser::decodeFile(LibMir::configFile(LibMir::config_file::NETCDF));
    standard_names.dump(Log::info()) << std::endl;
}


void Field::setMetadata(data::MIRField& mirField, size_t which) const {
    util::call_once(once, init);

    eckit::Value s = standard_names[standardName_];
    Log::info() << "NETCDF " << standardName_ << " => " << s << " " << s.isMap() << std::endl;

    if (s.isMap()) {
        eckit::ValueMap m = s;
        for (const auto& k : m) {
            mirField.metadata(which, k.first, k.second);
        }
    }
    else {
        Log::warning() << "No mapping for NetCDF standard name [" << standardName_ << "] " << variable_ << std::endl;
    }
}


}  // namespace netcdf
}  // namespace mir
