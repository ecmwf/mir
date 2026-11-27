// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include "mir/util/Search.h"

#include <map>

#include "eckit/geo/Grid.h"
#include "eckit/spec/Custom.h"

#include "mir/config/LibMir.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/repres/Representation.h"


namespace mir::util {


std::string search_loader(const std::string& name) {
    // mir's own k-d trees were mapped files (in the cache or temporary) or anonymous memory, kept for compatibility
    static const std::map<std::string, std::string> former{
        {"mapped-cache-file", "mapped-file"},
        {"mapped-temporary-file", "memory"},
        {"mapped-anonymous-memory", "memory"},
    };

    auto f = former.find(name);
    return f == former.end() ? name : f->second;
}


std::unique_ptr<const eckit::geo::Search> make_search(const param::MIRParametrisation& param,
                                                      const repres::Representation& rep) {
    using eckit::geo::Search;

    bool caching = LibMir::caching();
    param.get("caching", caching);

    std::string loader = caching ? LibMir::cacheLoader(LibMir::cache_loader::POINT_SEARCH) : "memory";
    param.get("point-search-trees", loader);
    loader = search_loader(loader);

    const auto& grid    = rep.canonicalGrid();
    const auto& library = Search::Storage::library();

    if (loader.empty() || loader == library.loader) {
        return std::make_unique<const Search>(grid.search(), eckit::spec::Custom{});
    }

    return std::make_unique<const Search>(grid, eckit::spec::Custom{}, Search::Storage{loader, library.files});
}


}  // namespace mir::util
