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


#include "mir/caching/WeightCache.h"

#include <sstream>

#include "eckit/config/Resource.h"

#include "mir/caching/matrix/MatrixLoader.h"
#include "mir/config/LibMir.h"
#include "mir/method/WeightMatrix.h"
#include "mir/param/MIRParametrisation.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Log.h"
#include "mir/util/Trace.h"


namespace mir {
namespace caching {


static std::string extract_loader(const param::MIRParametrisation& param) {
    std::string name = LibMir::cacheLoader(LibMir::cache_loader::MATRIX);
    param.get("matrix-loader", name);
    return name;
}


WeightCache::WeightCache(const param::MIRParametrisation& param) :
    eckit::CacheManager<WeightCacheTraits>(extract_loader(param), LibMir::cacheDir(),
                                           eckit::Resource<bool>("$MIR_THROW_ON_CACHE_MISS;mirThrowOnCacheMiss", false),
                                           eckit::Resource<size_t>("$MIR_MATRIX_CACHE_SIZE", 0)) {}


const char* WeightCacheTraits::name() {
    return "mir/weights";
}


int WeightCacheTraits::version() {
    return 17;
}


const char* WeightCacheTraits::extension() {
    return ".mat";
}


void WeightCacheTraits::save(const eckit::CacheManagerBase& /*unused*/, const value_type& W,
                             const eckit::PathName& path) {
    Log::debug() << "Inserting weights in cache : " << path << "" << std::endl;
    trace::Timer timer("Saving weights to cache");

    static size_t matrixMaxFootprint = eckit::Resource<size_t>("$MIR_MATRIX_MAX_FOOTPRINT", 0);
    if (matrixMaxFootprint > 0) {
        size_t size = W.footprint();
        if (size > matrixMaxFootprint) {
            std::ostringstream oss;
            oss << "WeightCacheTraits::save: matrix too large " << size << " (" << Log::Bytes(size) << "), maximum is "
                << Log::Bytes(matrixMaxFootprint);
            throw exception::UserError(oss.str());
        }
    }

    W.save(path);
}


void WeightCacheTraits::load(const eckit::CacheManagerBase& manager, value_type& w, const eckit::PathName& path) {
    trace::Timer timer("Loading weights from cache");

    value_type tmp(matrix::MatrixLoaderFactory::build(manager.loader(), path));
    w.swap(tmp);

    w.validate("fromCache");  // check matrix structure (only)
}


}  // namespace caching
}  // namespace mir
