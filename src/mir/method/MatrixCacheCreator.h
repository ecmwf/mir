// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


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
