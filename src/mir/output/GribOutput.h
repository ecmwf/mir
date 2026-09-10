// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include "mir/output/MIROutput.h"


struct grib_info;
struct grib_handle;


namespace mir::grib {
class Config;
}


namespace mir::output {


class GribOutput : public MIROutput {
public:
    // -- Constructors

    GribOutput();

    // -- Destructor

    ~GribOutput() override;

    // -- Methods

    virtual size_t interpolated() const;
    virtual size_t saved() const;

    // -- Class methods

    static const grib::Config& config();
    static param::MIRParametrisation* make_parametrised_config(const param::MIRParametrisation&);

    static bool do_save_with_metkit(const param::MIRParametrisation&);

protected:
    // -- Methods

    virtual void fill(grib_handle* handle, grib_info& info) const;

private:
    // -- Members

    size_t interpolated_;
    size_t saved_;

    // -- Methods

    virtual void out(const void* message, size_t length, bool interpolated) = 0;

    size_t save_with_metkit(const param::MIRParametrisation&, context::Context&);

    // -- Overridden methods

    // From MIROutput
    size_t copy(const param::MIRParametrisation&, context::Context&) override;
    size_t save(const param::MIRParametrisation&, context::Context&) override;
    size_t set(const param::MIRParametrisation&, context::Context&) override;
    bool sameParametrisation(const param::MIRParametrisation&, const param::MIRParametrisation&) const override;
    bool printParametrisation(std::ostream&, const param::MIRParametrisation&) const override;
    void prepare(const param::MIRParametrisation&, action::ActionPlan&, MIROutput&) override;
};


}  // namespace mir::output
