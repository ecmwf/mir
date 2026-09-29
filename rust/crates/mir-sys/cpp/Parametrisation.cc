// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "Parametrisation.h"

#include <sstream>

#include "eckit/log/JSON.h"


namespace mir_bridge {


rust::String Parametrisation::to_json() const {
    std::ostringstream s;
    eckit::JSON j(s);
    json(j);
    return rust::String(s.str());
}


std::unique_ptr<Parametrisation> Parametrisation::make() {
    return std::make_unique<Parametrisation>();
}


}  // namespace mir_bridge
