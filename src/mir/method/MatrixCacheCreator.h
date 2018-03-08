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
/// @date May 2015


#ifndef mir_method_MatrixCacheCreator_H
#define mir_method_MatrixCacheCreator_H

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

class MatrixCacheCreator : public caching::WeightCache::CacheContentCreator {

public:

  MatrixCacheCreator(const MethodWeighted & owner,
                     context::Context & ctx,
                     const repres::Representation & in,
                     const repres::Representation & out,
                     const lsm::LandSeaMasks & masks,
                     const Cropping& cropping);


private:

  virtual void create(const eckit::PathName & path,
                      WeightMatrix & W,
                      bool& saved) ;

private:

  const MethodWeighted& owner_;
  context::Context& ctx_;
  const repres::Representation& in_;
  const repres::Representation& out_;
  const lsm::LandSeaMasks& masks_;
  const Cropping& cropping_;

};


}  // namespace method
}  // namespace mir


#endif

