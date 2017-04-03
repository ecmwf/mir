/*
 * (C) Copyright 1996-2015 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Baudouin Raoult
/// @author Pedro Maciel
/// @date Apr 2015


#include "mir/action/transform/mapping/Table.h"

#include <iostream>
#include "eckit/config/EtcTable.h"
#include "eckit/exception/Exceptions.h"
#include "eckit/filesystem/PathName.h"
#include "eckit/memory/ScopedPtr.h"
#include "eckit/thread/AutoLock.h"
#include "eckit/types/FloatCompare.h"
#include "eckit/utils/Translator.h"
#include "mir/config/LibMir.h"
#include "mir/namedgrids/NamedGrid.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/repres/other/UnstructuredGrid.h"


namespace mir {
namespace action {
namespace transform {
namespace mapping {


namespace {


static MappingBuilder< Table > __mapping("table");


static pthread_once_t once = PTHREAD_ONCE_INIT;
static eckit::Mutex* local_mutex = 0;

static eckit::ScopedPtr<eckit::EtcKeyTable> table_n_to_truncation;
static eckit::ScopedPtr<eckit::EtcKeyTable> table_truncation_to_n;

static void init() {
    local_mutex = new eckit::Mutex();
    table_n_to_truncation.reset(new eckit::EtcKeyTable("auto-resol.table", 0, "etc/mir"));
    table_truncation_to_n.reset(new eckit::EtcKeyTable("auto-gaussian.table", 0, "etc/mir"));
}


static double computeStep(const param::MIRParametrisation& parametrisation) {
    double step = 0;

    std::vector<double> grid;
    if (parametrisation.get("user.grid", grid)) {
        ASSERT(grid.size() == 2);
        step = grid[1];
    }

    long N = 0;
    if (parametrisation.get("user.reduced", N)) {
        step = 90.0 / N;
    }

    if (parametrisation.get("user.regular", N)) {
        step = 90.0 / N;
    }

    if (parametrisation.get("user.octahedral", N)) {
        step = 90.0 / N;
    }

    std::string gridname;
    if (parametrisation.get("user.gridname", gridname)) {
        N = long(namedgrids::NamedGrid::lookup(gridname).gaussianNumber());
        step = 90.0 / N;
    }

    std::string griddef;
    if (parametrisation.get("user.griddef", griddef)) {
        // eckit::PathName path(griddef);
        // repres::other::UnstructuredGrid grid(path);
        // step = grid.increment();
        throw eckit::SeriousBug("AutoResol: computeStep does not yet support unstuctured grids");
    }

    return step;
}


eckit::Translator< std::string, double > stringToStep;
eckit::Translator< std::string, size_t > stringToNumber;
eckit::Translator< size_t, std::string > numberToString;


} // (anonymous namespace)


Table::Table(const param::MIRParametrisation& parametrisation) : Mapping(parametrisation) {
}


void Table::print(std::ostream& out) const {
    out << "<Table>";
}


size_t Table::getTruncationFromPointsPerLatitude(const size_t& N) const {
    pthread_once(&once, init);
    eckit::AutoLock<eckit::Mutex> lock(local_mutex);
    ASSERT(table_n_to_truncation);
    eckit::Log::debug<LibMir>() << "Table::getTruncationFromPointsPerLatitude(" << N << "):" << std::endl;

    // calculate a reference step (grid increment)
    double step = computeStep(parametrisation_);


    // pick entry with highest resolution (where step is not below the table's step entries)
    size_t T = 0;

    std::vector<std::string> steps = table_n_to_truncation->keys();
    for (std::vector<std::string>::const_iterator j = steps.begin(); j != steps.end(); ++j) {
        if (eckit::types::is_approximately_greater_or_equal<double>(step, stringToStep(*j))) {

            const std::vector<std::string>& entry = table_n_to_truncation->lookUp(*j);
            ASSERT(entry.size() == 2);
            T = stringToNumber(entry.back());

        }
    }

    if (!T) {
        std::ostringstream oss;
        oss << "Table::getTruncationFromPointsPerLatitude(" << N << "): cannot establish truncation for step " << step;
        throw eckit::SeriousBug(oss.str());
    }


    size_t truncation;
    bool limited = (get("field.truncation", truncation) && (truncation < T));
    if (limited) {
        T = truncation;
    }


    eckit::Log::debug<LibMir>() << "Table::getTruncationFromPointsPerLatitude(" << N << ") maps to truncation " << T << (limited? " (input limited)":"") << std::endl;
    return T;
}


size_t Table::getPointsPerLatitudeFromTruncation(const std::size_t& T) const {
    pthread_once(&once, init);
    eckit::AutoLock<eckit::Mutex> lock(local_mutex);
    ASSERT(table_truncation_to_n);
    eckit::Log::debug<LibMir>() << "Table::getPointsPerLatitudegeFromTruncation(" << T << "):" << std::endl;


    // ensure field.truncation converts to long
    size_t N = 0;
    ASSERT(T > 1);

    const std::vector<std::string>& entry = table_truncation_to_n->lookUp(numberToString(T));
    N = entry.size() < 2? 0 : stringToNumber(entry.back());

    if (!N) {
        std::ostringstream oss;
        oss << "Table: cannot establish N for truncation " << T;
        throw eckit::SeriousBug(oss.str());
    }


    eckit::Log::debug<LibMir>() << "Table::getPointsPerLatitudegeFromTruncation(" << T << "): truncation " << T << " maps to N " << N << std::endl;
    return N;
}


}  // namespace mapping
}  // namespace transform
}  // namespace action
}  // namespace mir

