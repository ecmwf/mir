/*
 * (C) Copyright 1996-2017 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#ifndef mir_method_AddParallelEdgesConnectivity_h
#define mir_method_AddParallelEdgesConnectivity_h


namespace atlas {
class Mesh;
}
namespace mir {
namespace util {
class Domain;
}
}


namespace mir {     // actually should be namespace atlas::mesh::actions
namespace method {  // ...


/// Adds traiangles connecting the poles and parallel edges to the mesh
class AddParallelEdgesConnectivity {
public:
    void operator()(const util::Domain&, atlas::Mesh&) const;
};


}  // namespace method
}  // namespace mir


#endif
