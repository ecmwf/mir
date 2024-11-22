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

#include "mir/output/MIROutput.h"


namespace eckit {
class PathName;
}


namespace mir::output {


class GriddefOutput : public MIROutput {
public:
    static void save(const eckit::PathName&, const std::vector<double>& latitudes,
                     const std::vector<double>& longitudes, bool binary = true);

protected:
    GriddefOutput(std::string path, bool binary);

private:
    const std::string path_;
    const bool binary_;

    size_t save(const param::MIRParametrisation&, context::Context&) override;
    bool sameAs(const MIROutput&) const override;
    bool sameParametrisation(const param::MIRParametrisation&, const param::MIRParametrisation&) const override;
    bool printParametrisation(std::ostream&, const param::MIRParametrisation&) const override;
    void print(std::ostream&) const override;
};


class GriddefTextOutput : public GriddefOutput {
public:
    GriddefTextOutput(std::string path) : GriddefOutput(path, false) {}
};


class GriddefBinaryOutput : public GriddefOutput {
public:
    GriddefBinaryOutput(std::string path) : GriddefOutput(path, true) {}
};


}  // namespace mir::output
