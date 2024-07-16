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


#include "mir/util/CheckDuplicatePoints.h"

#include <set>
#include <sstream>
#include <utility>

#include "eckit/config/Resource.h"

#include "mir/param/SimpleParametrisation.h"
#include "mir/util/Exceptions.h"


namespace mir::util {


void check_duplicate_points(const std::string& title, const std::vector<double>& latitudes,
                            const std::vector<double>& longitudes) {
    param::SimpleParametrisation empty;
    check_duplicate_points(title, latitudes, longitudes, empty);
}


void check_duplicate_points(const std::string& title, const std::vector<double>& latitudes,
                            const std::vector<double>& longitudes, const param::MIRParametrisation& parametrisation) {
    static const bool check = eckit::Resource<bool>("$MIR_CHECK_DUPLICATE_POINTS", true);

    bool checkDuplicatePoints = check;
    parametrisation.get("checkDuplicatePoints", checkDuplicatePoints);

    if (!checkDuplicatePoints) {
        return;
    }

    ASSERT(latitudes.size() == longitudes.size());
    ASSERT(!longitudes.empty());

    std::set<std::pair<double, double>> seen;
    size_t count = latitudes.size();

    for (size_t i = 0; i < count; ++i) {
        std::pair<double, double> p(latitudes[i], longitudes[i]);
        if (!seen.insert(p).second) {
            std::ostringstream oss;
            oss << title << ": duplicate point lat=" << latitudes[i] << ", lon=" << longitudes[i];
            throw exception::UserError(oss.str());
        }
    }
}


}  // namespace mir::util
