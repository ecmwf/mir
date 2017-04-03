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


#include "mir/action/transform/mapping/AutoGaussian.h"

#include <iostream>
#include "eckit/config/EtcTable.h"
#include "eckit/exception/Exceptions.h"
#include "eckit/memory/ScopedPtr.h"
#include "eckit/thread/AutoLock.h"
#include "eckit/utils/Translator.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/config/LibMir.h"


namespace mir {
namespace action {
namespace transform {
namespace mapping {


namespace  {


static pthread_once_t once = PTHREAD_ONCE_INIT;
static eckit::Mutex* local_mutex = 0;

static eckit::ScopedPtr<eckit::EtcKeyTable> table;

static void init() {
    local_mutex = new eckit::Mutex();
    table.reset(new eckit::EtcKeyTable("auto-gaussian.table", 0, "etc/mir"));
}


}  // (anonymous namespace)


AutoGaussian::AutoGaussian(const param::MIRParametrisation& parametrisation) :
    parametrisation_(parametrisation) {
}


AutoGaussian::~AutoGaussian() {
}


void AutoGaussian::get(const std::string& name, long& value) const {
    pthread_once(&once, init);
    eckit::AutoLock<eckit::Mutex> lock(local_mutex);
    ASSERT(table);

    eckit::Log::debug<LibMir>() << "AutoGaussian::get(" << name << ")" << std::endl;
    ASSERT(name == "octahedral"); // For now


    // ensure field.truncation converts to long
    long T = 0;
    ASSERT(parametrisation_.get("field.truncation", T));
    ASSERT(T > 1);

    value = T+1;
    return;

    eckit::Translator< std::string, long > stringToNumber;
    eckit::Translator< long, std::string > numberToString;

    const std::vector<std::string>& entry = table->lookUp(numberToString(T));
    value = entry.size() < 2? 0 : stringToNumber(entry.back());

    if (!value) {
        std::ostringstream oss;
        oss << "AutoGaussian: cannot establish N for truncation " << T;
        throw eckit::SeriousBug(oss.str());
    }


    eckit::Log::debug<LibMir>() << "AutoGaussian::get(" << name << "): truncation " << T << " maps to N " << value << std::endl;
}


void AutoGaussian::print(std::ostream& out) const {
    out << "<AutoGaussian>";
}


}  // namespace mapping
}  // namespace transform
}  // namespace action
}  // namespace mir

