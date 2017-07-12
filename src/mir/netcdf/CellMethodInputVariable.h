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

#ifndef mir_netcdf_CellMethodInputVariable
#define mir_netcdf_CellMethodInputVariable

#include "mir/netcdf/InputVariable.h"


namespace mir {
namespace netcdf {

class CellMethodInputVariable : public InputVariable {
public:

    CellMethodInputVariable(Dataset &owner, const std::string &name, int id, const std::vector<Dimension *> &dimensions);
    virtual ~CellMethodInputVariable();

private:

    // From InputVariable

    Variable *makeOutputVariable(Dataset &owner, const std::string &name, const std::vector<Dimension *> &dimensions) const ;

    // From variable

    virtual void print(std::ostream &s) const;
    const char* kind() const;

};

}
}
#endif
