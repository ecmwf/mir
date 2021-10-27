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


#include "mir/util/Log.h"

#include <utility>
#include <ostream>
#include <thread>
#include <sstream>
#include <fstream>

#include "eckit/log/BigNum.h"

#include "mir/config/LibMir.h"


namespace mir {


Log::Channel& Log::debug() {

    std::ostringstream oss;
    oss << "mirout-" << std::this_thread::get_id() << ".log";
    thread_local std::ofstream outfile{oss.str()};
    return outfile;
//    return eckit::Log::debug<LibMir>();
}


void Log::Pretty::print(std::ostream& s) const {
    s << eckit::BigNum(count_);
    if (plural_) {
        s << ' ' << plural_(count_);
    }
}


}  // namespace mir
