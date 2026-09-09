// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


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

    virtual bool sameAs(const NonLinear&) const                   = 0;
    virtual void hash(eckit::MD5&) const                          = 0;
    virtual bool modifiesMatrix(bool fieldHasMissingValues) const = 0;

    virtual const std::string& name() const = 0;
    virtual void json(eckit::JSON&) const { /* do nothing */ }

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
    NonLinearFactory& operator=(const NonLinearFactory&) = delete;
    NonLinearFactory& operator=(NonLinearFactory&&)      = delete;

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
