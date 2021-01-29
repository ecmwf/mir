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


#ifndef mir_util_Log_h
#define mir_util_Log_h


#include "eckit/log/Log.h"


namespace mir {


struct Log final : protected eckit::Log {
    using Channel = decltype(Log::debug());
    static Channel& debug();

    using eckit::Log::error;
    using eckit::Log::info;
    using eckit::Log::warning;

    using eckit::Log::applicationFormat;
};


}  // namespace mir


#endif
