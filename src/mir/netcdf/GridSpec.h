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

#include <iosfwd>
#include <vector>

#include "mir/util/Types.h"


namespace mir {
namespace netcdf {
class Variable;
}
}  // namespace mir


namespace mir {
namespace netcdf {


class GridSpec {
public:
    GridSpec(const Variable&);
    virtual ~GridSpec();

    GridSpec(const GridSpec&)       = delete;
    void operator=(const GridSpec&) = delete;

    // -- Methods

    static GridSpec* create(const Variable&);

    // For MIR
    virtual bool has(const std::string& name) const                             = 0;
    virtual bool get(const std::string&, long&) const                           = 0;
    virtual bool get(const std::string&, std::string&) const                    = 0;
    virtual bool get(const std::string& name, double& value) const              = 0;
    virtual bool get(const std::string& name, std::vector<double>& value) const = 0;

    virtual void reorder(MIRValuesVector& values) const = 0;


protected:
    // -- Members
    const Variable& variable_;

private:
    // From Endowed

    // - Methods

    virtual void print(std::ostream&) const = 0;

    // -- Friends
    friend std::ostream& operator<<(std::ostream& s, const GridSpec& v) {
        v.print(s);
        return s;
    }
};


class GridSpecGuesser {
    GridSpecGuesser(const GridSpecGuesser&)            = delete;
    GridSpecGuesser& operator=(const GridSpecGuesser&) = delete;

public:
    static GridSpec* guess(const Variable&);

protected:
    GridSpecGuesser(size_t priority);
    virtual ~GridSpecGuesser();

    virtual GridSpec* guess(const Variable& variable, const Variable& latitudes, const Variable& longitudes) const = 0;

private:
    size_t priority_;
};


template <class T>
class GridSpecGuesserBuilder : public GridSpecGuesser {
    GridSpec* guess(const Variable& variable, const Variable& latitudes, const Variable& longitudes) const override {
        return T::guess(variable, latitudes, longitudes);
    }

public:
    GridSpecGuesserBuilder(size_t priority) : GridSpecGuesser(priority) {}
};


}  // namespace netcdf
}  // namespace mir
