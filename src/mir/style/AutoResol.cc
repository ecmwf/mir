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


#include "mir/style/AutoResol.h"

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
namespace style {


namespace {


static pthread_once_t once = PTHREAD_ONCE_INIT;
static eckit::Mutex* local_mutex = 0;

static eckit::ScopedPtr<eckit::EtcKeyTable> table;

static void init() {
    local_mutex = new eckit::Mutex();
    table.reset(new eckit::EtcKeyTable("auto-resol.table", 0, "etc/mir"));
}


static double computeStep(const param::MIRParametrisation& parametrisation) {
    double step = 0;

    std::vector<double> grid;
    if (parametrisation.get("user.grid", grid)) {
        ASSERT(grid.size() == 2);
        step = std::min(grid[0], grid[1]);
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


} // (anonymous namespace)


AutoResol::AutoResol(const param::MIRParametrisation& parametrisation) :
    parametrisation_(parametrisation) {
}


AutoResol::~AutoResol() {
}


void AutoResol::get(const std::string& name, long& value) const {
    pthread_once(&once, init);
    eckit::AutoLock<eckit::Mutex> lock(local_mutex);
    ASSERT(table);

    eckit::Log::debug<LibMir>() << "AutoResol::get(" << name << ")" << std::endl;
    ASSERT(name == "truncation"); // For now


    // calculate a reference step (grid increment)
    double step = computeStep(parametrisation_);
    if (!eckit::types::is_strictly_greater<double>(step, 0)) {
        std::ostringstream oss;
        oss << "AutoResol: cannot compute grid step from parametrisation (step = " << step << ")";
        throw eckit::SeriousBug(oss.str());
    }


    eckit::Translator< std::string, double > stringToStep;
    eckit::Translator< std::string, long > stringToNumber;


    // pick entry with highest resolution (where step is not below the table's step entries)
    value = 0;

    std::vector<std::string> steps = table->keys();
    for (std::vector<std::string>::const_iterator j = steps.begin(); j != steps.end(); ++j) {
        if (eckit::types::is_approximately_greater_or_equal<double>(step, stringToStep(*j))) {

            const std::vector<std::string>& entry = table->lookUp(*j);
            ASSERT(entry.size() == 2);
            value = stringToNumber(entry.back());

        }
    }

    if (!value) {
        std::ostringstream oss;
        oss << "AutoResol::get(" << name << "): cannot establish truncation for step " << step;
        throw eckit::SeriousBug(oss.str());
    }


    long truncation;
    bool limited = (parametrisation_.get("field.truncation", truncation) && (truncation < value));
    if (limited) {
        value = truncation;
    }


    eckit::Log::debug<LibMir>() << "AutoResol::get(" << name << ") step " << step << " maps to truncation " << value << (limited? " (input limited)":"") << std::endl;
}

void AutoResol::print(std::ostream& out) const {
    out << "<AutoResol>";
}

} // namespace param
} // namespace mir
