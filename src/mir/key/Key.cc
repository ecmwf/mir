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


#include "mir/key/Key.h"

#include "eckit/config/Configuration.h"

#include "mir/config/LibMir.h"


namespace mir {
namespace key {


Key::keywords_t Key::postProcess() {
    static const keywords_t defaultKeywords{
        "accuracy", "bitmap",  "checkerboard", "compatibility", "edition", "filter",  "format",  "formula",
        "frame",    "griddef", "latitudes",    "longitudes",    "nabla",   "packing", "pattern", "vod2uv",
    };

    auto& config = LibMir::instance().configuration();

    static const keywords_t keywords = config.getStringVector("post-process", defaultKeywords);

    return keywords;
}


}  // namespace key
}  // namespace mir
