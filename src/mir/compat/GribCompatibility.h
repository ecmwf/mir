// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include <iosfwd>
#include <map>
#include <string>


struct grib_info;
struct grib_handle;

namespace metkit::mars {
class MarsRequest;
}  // namespace metkit::mars

namespace mir {
namespace param {
class MIRParametrisation;
}
namespace output {
class MIROutput;
}
}  // namespace mir


namespace mir::compat {


class GribCompatibility {
public:
    GribCompatibility(const GribCompatibility&) = delete;
    GribCompatibility(GribCompatibility&&)      = delete;

    void operator=(const GribCompatibility&) = delete;
    void operator=(GribCompatibility&&)      = delete;

    virtual void execute(const output::MIROutput&, const param::MIRParametrisation&, grib_handle*,
                         grib_info&) const = 0;

    virtual void printParametrisation(std::ostream&, const param::MIRParametrisation&) const = 0;

    virtual bool sameParametrisation(const param::MIRParametrisation&, const param::MIRParametrisation&) const = 0;

    virtual void initialise(const metkit::mars::MarsRequest&, std::map<std::string, std::string>& postproc) const = 0;

    static const GribCompatibility& lookup(const std::string&);

    static void list(std::ostream&);

protected:
    GribCompatibility(const std::string&);
    virtual ~GribCompatibility();

    virtual void print(std::ostream&) const = 0;

private:
    std::string name_;

    friend std::ostream& operator<<(std::ostream& s, const GribCompatibility& p) {
        p.print(s);
        return s;
    }
};


}  // namespace mir::compat
