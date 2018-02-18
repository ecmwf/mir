/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#ifndef mir_util_AddParallelEdgesConnectivity_h
#define mir_util_AddParallelEdgesConnectivity_h

#include "mir/util/Types.h"


namespace atlas {
class Mesh;
}


namespace mir {     // actually should be namespace atlas::mesh::actions
namespace util {  // ...


/// Adds triangles connecting the poles and parallel edges to the mesh
class AddParallelEdgesConnectivity {
public:
    void operator()(atlas::Mesh&, const Latitude& north, const Latitude& south) const;
};


}  // namespace util
}  // namespace mir


#endif
