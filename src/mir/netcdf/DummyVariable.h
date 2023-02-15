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

#include "mir/netcdf/Variable.h"


namespace mir::netcdf {


class DummyVariable : public Variable {
public:
    DummyVariable(const Variable& parent);
    ~DummyVariable() override;

protected:
    const Variable& parent_;

    // -- Methods

    virtual Variable* makeOutputVariable(Dataset& owner, const std::string& name,
                                         const std::vector<Dimension*>& dimensions) const;
    virtual bool dummy() const;
    virtual bool sameAsDummy(const Variable&) const;

    // From variable

    void print(std::ostream&) const override;

    // From Endowed

    virtual int varid() const;
};


}  // namespace mir::netcdf
