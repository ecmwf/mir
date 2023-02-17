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


#pragma once

#include "mir/caching/WeightCache.h"


namespace eckit {
class PathName;
}

namespace mir {
namespace context {
class Context;
}
namespace repres {
class Representation;
}
namespace lsm {
class LandSeaMasks;
}
namespace method {
class Cropping;
class MethodWeighted;
}  // namespace method
}  // namespace mir


namespace mir::method {


class MatrixCacheCreator : public caching::WeightCache::CacheContentCreator {

    void create(const eckit::PathName& path, WeightMatrix& W, bool& saved) override;

public:
    MatrixCacheCreator(const MethodWeighted& owner, context::Context&, const repres::Representation& in,
                       const repres::Representation& out, const lsm::LandSeaMasks&, const Cropping&);
    ~MatrixCacheCreator() = default;

    MatrixCacheCreator(const MatrixCacheCreator&)            = delete;
    MatrixCacheCreator(MatrixCacheCreator&&)                 = delete;
    MatrixCacheCreator& operator=(const MatrixCacheCreator&) = delete;
    MatrixCacheCreator& operator=(MatrixCacheCreator&&)      = delete;

private:
    const MethodWeighted& owner_;
    context::Context& ctx_;
    const repres::Representation& in_;
    const repres::Representation& out_;
    const lsm::LandSeaMasks& masks_;
    const Cropping& cropping_;
};


}  // namespace mir::method
