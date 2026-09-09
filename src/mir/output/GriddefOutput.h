// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


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
