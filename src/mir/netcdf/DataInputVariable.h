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


#ifndef mir_netcdf_DataInputVariable
#define mir_netcdf_DataInputVariable

#include "mir/netcdf/InputVariable.h"


namespace mir {
namespace netcdf {

class DataInputVariable : public InputVariable {
public:
    DataInputVariable(Dataset &owner, const std::string &name, int id, const std::vector<Dimension *> &dimensions);
    virtual ~DataInputVariable();
private:

    std::vector<const Variable*> coordinates_;


    Variable *makeOutputVariable(Dataset &owner, const std::string &name, const std::vector<Dimension *> &dimensions) const ;
    virtual void print(std::ostream &s) const;
    virtual const std::string &ncname() const;
    virtual void collectField(std::vector<Field *>&) const;

    virtual size_t count2DValues() const;
    virtual void get2DValues(MIRValuesVector&, size_t i) const;

    // Used during identtification
    virtual void addCoordinateVariable(const Variable*);
    virtual Variable* addMissingCoordinates();
    const char* kind() const;
    void dumpAttributes(std::ostream &s, const char* prefix) const;

    virtual std::vector<std::string> coordinates() const;

    mutable std::string ncname_;

};


}  // namespace netcdf
}  // namespace mir


#endif
