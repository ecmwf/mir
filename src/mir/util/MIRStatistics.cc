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


#include "mir/util/MIRStatistics.h"

#include <sstream>
#include <utility>
#include <vector>

#include "eckit/log/JSON.h"
#include "eckit/serialisation/Stream.h"


namespace mir::util {


static const std::vector<std::string> all_caches{"mirBitmap", "mirArea", "mirCoefficient", "mirMatrix", "mirMesh"};


static const std::vector<std::pair<std::string, std::string>> all_timings{
    {"crop", "Time in area-crop"},
    {"frame", "Time in extracting frames"},
    {"globalise", "Time in extending to globe"},
    {"bitmap", "Time applying bitmaps"},
    {"coefficient", "Time loading/building coeff."},
    {"sh2grid", "Time in SH to grid transform"},
    {"grid2grid", "Time in grid to grid interp."},
    {"vod2uv", "Time in VO/D to U/V"},
    {"computeMatrix", "Time compute matrices"},
    {"matrix", "Time matrix multiply"},
    {"loadCoeff", "Time loading coefficients"},
    {"createCoeff", "Time creating coefficients"},
    {"calc", "Time in basic computations"},
    {"nabla", "Time in nabla calculations"},
    {"save", "Time saving"},
    {"gribEncoding", "Time in GRIB encoding"},
    {"gribDecoding", "Time in GRIB decoding"}};


MIRStatistics::MIRStatistics() {
    for (const auto& c : all_caches) {
        caches_.insert({c, {}});
    }

    for (const auto& td : all_timings) {
        timings_.insert({td.first, {}});
        descriptions_[td.first] = td.second;
    }
}


MIRStatistics::MIRStatistics(eckit::Stream& s) {
    for (const auto& c : all_caches) {
        caches_.insert({c, s});
    }

    for (const auto& td : all_timings) {
        s >> timings_[td.first];
        descriptions_[td.first] = td.second;
    }
}


void MIRStatistics::encode(eckit::Stream& s) const {
    for (const auto& cache : caches_) {
        s << cache.second;
    }

    for (const auto& tim : timings_) {
        s << tim.second;
    }
}


void MIRStatistics::json(eckit::JSON& j) const {
    j.startObject();

    for (const auto& cache : caches_) {
        std::ostringstream s;
        s << cache.second.footprint_;
        j << cache.first << s.str();
    }

    for (const auto& tim : timings_) {
        j << tim.first << tim.second.elapsed_;
    }

    j.endObject();
}


MIRStatistics& MIRStatistics::operator+=(const MIRStatistics& other) {
    for (auto& cache : caches_) {
        cache.second += other.caches_.at(cache.first);
    }

    for (auto& tim : timings_) {
        tim.second += other.timings_.at(tim.first);
    }

    return *this;
}


MIRStatistics& MIRStatistics::operator/=(size_t n) {
    for (auto& cache : caches_) {
        cache.second /= n;
    }

    for (auto& tim : timings_) {
        tim.second /= n;
    }

    return *this;
}


void MIRStatistics::report(std::ostream& out, const char* indent) const {
    for (const auto& cache : caches_) {
        cache.second.report(cache.first.c_str(), out, indent);
    }

    for (const auto& tim : timings_) {
        auto description = descriptions_.at(tim.first);
        reportTime(out, description.c_str(), tim.second, indent);
    }
}


void MIRStatistics::csvHeader(std::ostream& out) const {
    const auto* sep = "";
    for (const auto& tim : timings_) {
        out << sep << tim.first;
        sep = ",";
    }
}


void MIRStatistics::csvRow(std::ostream& out) const {
    const auto* sep = "";
    for (const auto& tim : timings_) {
        out << sep << tim.second;
        sep = ", ";
    }
}


}  // namespace mir::util
