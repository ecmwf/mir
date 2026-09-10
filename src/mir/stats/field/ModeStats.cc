// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/stats/field/ModeStats.h"

#include <ostream>

#include "eckit/log/JSON.h"

#include "mir/stats/detail/ModeT.h"


namespace mir::stats::field {


struct ModeReal final : ModeStatsT<detail::ModeReal> {
    using ModeStatsT::ModeStatsT;
    double value() const override { return mode(); }
    void json(eckit::JSON& j) const override { json_tv(j, "mode-real", value()); }
    void print(std::ostream& out) const override { out << "ModeReal[" << value() << "]"; }
};


struct ModeIntegral final : ModeStatsT<detail::ModeIntegral> {
    using ModeStatsT::ModeStatsT;
    double value() const override { return mode(); }
    void json(eckit::JSON& j) const override { json_tv(j, "mode-integral", value()); }
    void print(std::ostream& out) const override { out << "ModeIntegral[" << value() << "]"; }
};


struct ModeBoolean final : ModeStatsT<detail::ModeBoolean> {
    using ModeStatsT::ModeStatsT;
    double value() const override { return mode(); }
    void json(eckit::JSON& j) const override { json_tv(j, "mode-boolean", value()); }
    void print(std::ostream& out) const override { out << "ModeBoolean[" << value() << "]"; }
};


struct MedianIntegral final : ModeStatsT<detail::MedianIntegral> {
    using ModeStatsT::ModeStatsT;
    double value() const override { return median(); }
    void json(eckit::JSON& j) const override { json_tv(j, "median-integral", value()); }
    void print(std::ostream& out) const override { out << "MedianIntegral[" << value() << "]"; }
};


static const FieldBuilder<ModeReal> __builder_1("mode-real");
static const FieldBuilder<ModeIntegral> __builder_2("mode-integral");
static const FieldBuilder<ModeBoolean> __builder_3("mode-boolean");

static const FieldBuilder<MedianIntegral> __builder_4("median-integral");
static const FieldBuilder<ModeBoolean> __builder_5("median-boolean");  // (alias)


}  // namespace mir::stats::field
