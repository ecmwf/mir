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


#include "mir/util/Wind.h"

#include "eckit/config/Configuration.h"

#include "mir/config/LibMir.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Log.h"


namespace mir {
namespace util {


constexpr long PARAMETER_ID_U  = 131;
constexpr long PARAMETER_ID_V  = 132;
constexpr long PARAMETER_ID_VO = 138;
constexpr long PARAMETER_ID_D  = 155;


Wind::Defaults::Defaults() :
    u(LibMir::instance().configuration().getLong("parameter-id-u", PARAMETER_ID_U)),
    v(LibMir::instance().configuration().getLong("parameter-id-v", PARAMETER_ID_V)),
    vo(LibMir::instance().configuration().getLong("parameter-id-vo", PARAMETER_ID_VO)),
    d(LibMir::instance().configuration().getLong("parameter-id-d", PARAMETER_ID_D)) {
    ASSERT(0 < u && u < 1000);
    ASSERT(0 < v && v < 1000);
    ASSERT(0 < vo && vo < 1000);
    ASSERT(0 < d && d < 1000);
}


void Wind::paramIds(const param::MIRParametrisation& parametrisation, long& u, long& v) {
    constexpr long THOUSAND = 1000;

    // User input if given
    bool need_u = !parametrisation.userParametrisation().get("paramId.u", u);
    bool need_v = !parametrisation.userParametrisation().get("paramId.v", v);

    if (need_u || need_v) {

        // assumes the same input parameter table for the defaults
        long id = 0;
        if (parametrisation.fieldParametrisation().get("paramId", id)) {
            ASSERT(id > 0);
        }

        long table = id / THOUSAND;

        static const Defaults def;

        if (need_u) {
            u = def.u + table * THOUSAND;
        }

        if (need_v) {
            v = def.v + table * THOUSAND;
        }
    }

    Log::debug() << "Wind: u/v = " << u << "/" << v << std::endl;
}


}  // namespace util
}  // namespace mir
