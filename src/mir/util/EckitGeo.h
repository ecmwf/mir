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

#include "mir/api/mir_config.h"

#if mir_HAVE_ECKIT_GEO

#include "eckit/geo/Area.h"
#include "eckit/geo/Grid.h"
#include "eckit/geo/Ordering.h"
#include "eckit/geo/Projection.h"
#include "eckit/geo/Spec.h"

#else


namespace eckit::geo {


enum Ordering
{
};


class Spec {};


}  // namespace eckit::geo


#endif
