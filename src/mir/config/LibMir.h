/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Baudouin Raoult
/// @author Tiago Quintino
/// @author Pedro Maciel
/// @date August 2016


#ifndef mir_config_LibMir_h
#define mir_config_LibMir_h

#include <string>
#include <vector>

#include "eckit/system/Library.h"


namespace mir {


class LibMir : public eckit::system::Library {
public:
    using keywords_t = std::vector<std::string>;

    LibMir();
    static const LibMir& instance();

    static std::string cacheDir();
    static bool caching();
    static const keywords_t postProcess();

protected:
    const void* addr() const;
    virtual std::string version() const;
    virtual std::string gitsha1(unsigned int count) const;
};


} // namespace mir


#endif
