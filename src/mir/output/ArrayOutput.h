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

#include <limits>
#include <vector>

#include "mir/output/MIROutput.h"


namespace mir::output {


class ArrayOutput final : public MIROutput {
public:
    ArrayOutput() = default;

    std::vector<double>& values() { return values_; }
    const std::vector<size_t>& shape() const { return shape_; }
    const std::string& gridspec() const { return gridspec_; }
    double missingValue() const { return missingValue_; }

    size_t size() const;

private:
    std::vector<double> values_;
    std::vector<size_t> shape_;
    std::string gridspec_;
    double missingValue_{std::numeric_limits<double>::quiet_NaN()};

    size_t save(const param::MIRParametrisation&, context::Context&) override;
    void print(std::ostream&) const override;

    bool sameAs(const MIROutput&) const override { return false; /*dummy*/ }
    bool sameParametrisation(const param::MIRParametrisation&, const param::MIRParametrisation&) const override {
        return false; /*dummy*/
    }
    bool printParametrisation(std::ostream&, const param::MIRParametrisation&) const override {
        return false; /*dummy*/
    }
};


}  // namespace mir::output
