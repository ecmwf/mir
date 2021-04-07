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

#include <cstddef>
#include <iosfwd>
#include <memory>
#include <string>

#include "mir/data/MIRValuesVector.h"


namespace mir {
namespace data {
class MIRField;
}
namespace netcdf {
class GridSpec;
class Variable;
}  // namespace netcdf
}  // namespace mir


namespace mir {
namespace netcdf {


class Field {
public:
    Field(const Variable&);
    ~Field();

    // -- Methods

    // std::vector<size_t> dimensions() const;

    size_t count2DValues() const;
    void get2DValues(MIRValuesVector&, size_t i) const;

    bool hasMissing() const;
    double missingValue() const;

    // For MIR
    bool has(const std::string& name) const;
    bool get(const std::string&, long&) const;
    bool get(const std::string&, std::string&) const;
    bool get(const std::string& name, double& value) const;
    bool get(const std::string& name, std::vector<double>& value) const;


    void setMetadata(data::MIRField& mirField, size_t which) const;

protected:
    // -- Members
    const Variable& variable_;
    std::string standardName_;
    std::string units_;

private:
    Field(const Field&);
    Field& operator=(const Field&);


    mutable std::unique_ptr<GridSpec> gridSpec_;

    // - Methods

    const GridSpec& gridSpec() const;


    void print(std::ostream&) const;

    // -- Friends
    friend std::ostream& operator<<(std::ostream& s, const Field& v) {
        v.print(s);
        return s;
    }
};


}  // namespace netcdf
}  // namespace mir
