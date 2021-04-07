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

#include <iostream>
#include <string>
#include <vector>

#include "mir/data/MIRValuesVector.h"
#include "mir/netcdf/Endowed.h"


namespace mir {
namespace netcdf {
class Dataset;
class Dimension;
class Matrix;
class MergePlan;
class Field;
}  // namespace netcdf
}  // namespace mir


namespace mir {
namespace netcdf {


class Variable : public Endowed {
public:
    Variable(Dataset& owner, const std::string& name, const std::vector<Dimension*>& dimensions);
    ~Variable() override;

    // -- Methods

    void setMatrix(Matrix*);
    size_t numberOfValues() const;
    virtual std::vector<std::string> coordinates() const;
    virtual std::vector<std::string> cellMethods() const;

    virtual bool identified() const;

    virtual bool coordinate() const;
    virtual bool scalar() const;

    virtual void validate() const;
    virtual bool sameAs(const Variable&) const;
    virtual void dump(std::ostream&) const;
    virtual void dumpData(std::ostream&) const;
    virtual void dumpAttributes(std::ostream&, const char* prefix) const;

    virtual void create(int nc) const;
    virtual void save(int nc) const;
    virtual Variable* clone(Dataset& owner) const;
    virtual void merge(const Variable&, MergePlan&);

    // From Endowed
    const std::string& name() const override;
    const std::string& path() const override;

    Matrix* matrix() const;

    const std::vector<Dimension*>& dimensions() const;

    virtual bool dummy() const;
    virtual bool sameAsDummy(const Variable&) const;

    virtual bool timeAxis() const;

    virtual void addVirtualDimension(size_t where, Dimension*);
    virtual Dimension* getVirtualDimension();
    bool sharesDimensions(const Variable&) const;

    virtual const std::string& ncname() const;
    virtual void collectField(std::vector<Field*>&) const;

    virtual void values(std::vector<double>& values) const;

    virtual const char* kind() const;

    const Dataset& dataset() const;
    Dataset& dataset();

    const Variable& lookupInDataset(const std::string& standardName, const std::string& units, size_t n) const;

    // Used during identtification
    virtual Variable* makeDataVariable();
    virtual Variable* makeCoordinateVariable();
    virtual Variable* makeSimpleVariable();
    virtual Variable* makeCellMethodVariable();
    virtual Variable* makeScalarCoordinateVariable();
    virtual void initCodecs();

    virtual void addCoordinateVariable(const Variable*);
    virtual Variable* addMissingCoordinates();

    bool hasAttribute(const std::string& name) const;

    template <class T>
    T getAttributeValue(const std::string& name) const {
        T result;
        // Log::info() << "getAttributeValue ==> " << name << std::endl;
        getAttribute(name, result);
        // Log::info() << "getAttributeValue <== " << result << std::endl;
        return result;
    }

    // CF
    virtual size_t numberOfDimensions() const;
    virtual size_t count2DValues() const;
    virtual void get2DValues(MIRValuesVector& values, size_t i) const;
    virtual bool hasMissing() const;
    virtual double missingValue() const;

protected:
    // -- Members

    Dataset& dataset_;
    std::string name_;
    Matrix* matrix_;
    std::vector<Dimension*> dimensions_;
    bool scalar_;

private:
    Variable(const Variable&);
    Variable& operator=(const Variable&);

    // -- Methods

    // CF
    void getAttribute(const std::string& name, std::string& value) const;
    void getAttribute(const std::string& name, double& value) const;

    virtual void print(std::ostream&) const = 0;

    // -- Friends
    //
    friend std::ostream& operator<<(std::ostream& s, const Variable& v) {
        v.print(s);
        return s;
    }
};


}  // namespace netcdf
}  // namespace mir
