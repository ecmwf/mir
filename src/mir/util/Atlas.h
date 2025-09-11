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

// #include <string>
// #include <vector>

// #include "eckit/config/LocalConfiguration.h"
// #include "eckit/geometry/Point2.h"
// #include "eckit/geometry/Point3.h"
// #include "eckit/geometry/SphereT.h"
// #include "eckit/utils/Hash.h"

// #include "mir/api/mir_config.h"
// #include "mir/util/Types.h"

#include "atlas/functionspace.h"
#include "atlas/grid.h"
#include "atlas/interpolation.h"
#include "atlas/interpolation/element/Quad3D.h"
#include "atlas/interpolation/element/Triag3D.h"
#include "atlas/interpolation/method/PointIndex3.h"
#include "atlas/interpolation/method/Ray.h"
#include "atlas/library/Library.h"
#include "atlas/library/config.h"
#include "atlas/mesh.h"
#include "atlas/mesh/actions/BuildCellCentres.h"
#include "atlas/mesh/actions/BuildNode2CellConnectivity.h"
#include "atlas/mesh/actions/BuildXYZField.h"
#include "atlas/meshgenerator.h"
#include "atlas/numerics/Nabla.h"
#include "atlas/numerics/fvm/Method.h"
#include "atlas/option.h"
#include "atlas/output/Gmsh.h"
#include "atlas/projection/detail/ProjectionFactory.h"
#include "atlas/trans/LegendreCacheCreator.h"
#include "atlas/trans/Trans.h"
#include "atlas/util/Config.h"
#include "atlas/util/Earth.h"
#include "atlas/util/GaussianLatitudes.h"
#include "atlas/util/Point.h"
#include "atlas/util/Rotation.h"


namespace mir::util {


using atlas::util::Earth;


}  // namespace mir::util
