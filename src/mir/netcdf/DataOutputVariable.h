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

#ifndef mir_netcdf_DataOutputVariable
#define mir_netcdf_DataOutputVariable

#include "mir/netcdf/OutputVariable.h"


namespace mir {
namespace netcdf {

class DataOutputVariable : public OutputVariable {
public:

    DataOutputVariable(Dataset &owner,
                       const std::string &name,
                       const std::vector<Dimension *> &dimensions);

    virtual ~DataOutputVariable();

private:

    virtual void merge(const Variable &, MergePlan &plan);
    virtual void print(std::ostream &s) const;
    virtual const std::string &ncname() const;
    virtual void collectField(std::vector<Field *>&) const;

    mutable std::string ncname_;
};

}
}
#endif
