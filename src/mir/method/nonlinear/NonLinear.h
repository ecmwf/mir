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
#include <string>

#include "mir/method/MethodWeighted.h"


namespace mir::method::nonlinear {


class NonLinear {
public:
    explicit NonLinear(const param::MIRParametrisation&);

    NonLinear(const NonLinear&)      = delete;
    NonLinear(NonLinear&&)           = delete;
    void operator=(NonLinear&&)      = delete;
    void operator=(const NonLinear&) = delete;

    virtual ~NonLinear();

    /// Update interpolation linear system to account for non-linearities
    virtual bool treatment(DenseMatrix& A, WeightMatrix& W, DenseMatrix& B, const MIRValuesVector&,
                           const double& missingValue) const = 0;

    virtual bool sameAs(const NonLinear&) const = 0;
    virtual void hash(eckit::MD5&) const        = 0;
    virtual void json(eckit::JSON&) const       = 0;

    virtual bool modifiesMatrix(bool fieldHasMissingValues) const = 0;

private:
    virtual void print(std::ostream&) const = 0;

    friend std::ostream& operator<<(std::ostream& s, const NonLinear& p) {
        p.print(s);
        return s;
    }

    friend eckit::JSON& operator<<(eckit::JSON& s, const NonLinear& p) {
        p.json(s);
        return s;
    }
};


class NonLinearFactory {
private:
    std::string name_;
    virtual NonLinear* make(const param::MIRParametrisation&) = 0;

protected:
    explicit NonLinearFactory(const std::string& name);
    virtual ~NonLinearFactory();

public:
    NonLinearFactory(const NonLinearFactory&)            = delete;
    NonLinearFactory(NonLinearFactory&&)                 = delete;
    NonLinearFactory& operator=(NonLinearFactory&&)      = delete;
    NonLinearFactory& operator=(const NonLinearFactory&) = delete;

    static const NonLinear* build(const std::string& name, const param::MIRParametrisation&);
    static void list(std::ostream&);
};


template <class T>
class NonLinearBuilder : public NonLinearFactory {
    NonLinear* make(const param::MIRParametrisation& param) override { return new T(param); }

public:
    explicit NonLinearBuilder(const std::string& name) : NonLinearFactory(name) {}
};


}  // namespace mir::method::nonlinear
