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


#include "mir/action/filter/StatisticsFilter.h"

#include <sstream>

#include "eckit/utils/StringTools.h"

#include "mir/action/context/Context.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/stats/Statistics.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Log.h"


namespace mir {
namespace action {
namespace filter {


StatisticsFilter::StatisticsFilter(const param::MIRParametrisation& param) : Action(param) {
    std::string which;
    param.get("which-statistics", which);

    std::string statistics = "scalar";
    ASSERT(param.get(which + "-statistics", statistics) || param.get("statistics", statistics));

    auto stats(eckit::StringTools::split("/", statistics));
    statistics_.reserve(stats.size());

    for (auto& s : stats) {
        statistics_.push_back(std::unique_ptr<stats::Statistics>(stats::StatisticsFactory::build(s, param)));
        ASSERT(statistics_.back());
    }

    param.get("precision", precision_ = size_t(Log::info().precision()));
}


bool StatisticsFilter::sameAs(const Action& other) const {
    const auto* o = dynamic_cast<const StatisticsFilter*>(&other);
    if ((o == nullptr) || (statistics_.size() != o->statistics_.size())) {
        return false;
    }

    // FIXME just a workaround while there is no stats::Statistics::sameAs
    std::ostringstream a;
    std::ostringstream b;
    for (size_t i = 0; i < statistics_.size(); ++i) {
        a.clear();
        a << *statistics_[i];

        b.clear();
        b << *(o->statistics_[i]);

        if (a.str() != b.str()) {
            return false;
        }
    }

    return true;
}


const char* StatisticsFilter::name() const {
    return "StatisticsFilter";
}


void StatisticsFilter::print(std::ostream& out) const {
    out << "StatisticsFilter[";
    const auto* sep = "";
    for (const auto& s : statistics_) {
        out << sep << (*s);
        sep = ",";
    }
    out << "]";
}


void StatisticsFilter::execute(context::Context& ctx) const {
    auto& log = Log::info();
    auto old  = log.precision(static_cast<decltype(log.precision())>(precision_));

    auto& field = ctx.field();
    for (const auto& s : statistics_) {
        s->execute(field);
        log << *s << std::endl;
    }

    log.precision(old);
}


static const ActionBuilder<StatisticsFilter> __action("filter.statistics");


}  // namespace filter
}  // namespace action
}  // namespace mir
