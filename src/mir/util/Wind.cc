/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */


#include "mir/util/Wind.h"

#include "eckit/log/Log.h"
#include "mir/config/LibMir.h"
#include "mir/param/MIRParametrisation.h"


namespace mir {
namespace util {


struct Defaults {
    Defaults() :
        u(LibMir::instance().configuration().getLong("parameter-id-u", 131)),
        v(LibMir::instance().configuration().getLong("parameter-id-v", 132)) {
        ASSERT(0 < u && u < 1000);
        ASSERT(0 < v && v < 1000);
    }
    const long u;
    const long v;
};


void Wind::paramIds(const param::MIRParametrisation& parametrisation, size_t& u, size_t& v) {

    // User input if given
    bool need_u = !parametrisation.userParametrisation().get("paramId.u", u);
    bool need_v = !parametrisation.userParametrisation().get("paramId.v", v);

    if (need_u || need_v) {

        // assumes the same input parameter table for the defaults
        size_t id = 0;
        ASSERT(parametrisation.fieldParametrisation().get("paramId", id));
        ASSERT(id > 0);

        size_t table = id / 1000;

        static const Defaults def;

        if (need_u) {
            u = size_t(def.u) + table * 1000;
        }

        if (need_v) {
            v = size_t(def.v) + table * 1000;
        }
    }

    eckit::Log::debug<LibMir>() << "Wind: u/v = " << u << "/" << v << std::endl;
}


bool Wind::isInputWind(const param::MIRParametrisation& parametrisation) {

    long id = 0;
    if (!parametrisation.fieldParametrisation().get("paramId", id) || !id) {
        return false;
    }

    static const Defaults def;

    return id == def.u || id == def.v;
}


bool Wind::isOutputWind(const param::MIRParametrisation& parametrisation) {

    if (isInputWind(parametrisation)) {
        return true;
    }

    bool vod2uv = false;
    parametrisation.userParametrisation().get("vod2uv", vod2uv);

    bool wind = false;
    parametrisation.userParametrisation().get("wind", wind);

    return vod2uv || wind;
}


}  // namespace util
}  // namespace mir

