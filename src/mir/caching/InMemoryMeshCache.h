// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include "mir/caching/InMemoryCache.h"


namespace atlas {
class Mesh;
class Grid;
}  // namespace atlas

namespace mir::util {
class MIRStatistics;
class MeshGeneratorParameters;
}  // namespace mir::util


namespace mir::caching {


class InMemoryMeshCache {
public:
    // -- Types
    // None

    // -- Exceptions
    // None

    // -- Constructors

    InMemoryMeshCache(const InMemoryMeshCache&) = delete;
    InMemoryMeshCache(InMemoryMeshCache&&)      = delete;

    // -- Destructor
    // None

    // -- Convertors
    // None

    // -- Operators

    void operator=(const InMemoryMeshCache&) = delete;
    void operator=(InMemoryMeshCache&&)      = delete;

    // -- Methods

    static const InMemoryMeshCache& instance();
    static atlas::Mesh atlasMesh(util::MIRStatistics&, const atlas::Grid&, const util::MeshGeneratorParameters&);

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

protected:
    // -- Members
    // None

    // -- Methods
    // None

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

private:
    // -- Constructors

    InMemoryMeshCache();

    // -- Destructor

    virtual ~InMemoryMeshCache() = default;

    // -- Operators
    // None

    // -- Members
    // None

    // -- Methods

    virtual void print(std::ostream&) const;

    // -- Overridden methods
    // None

    // -- Class members
    // None

    // -- Class methods
    // None

    // -- Friends

    friend std::ostream& operator<<(std::ostream& s, const InMemoryMeshCache& p) {
        p.print(s);
        return s;
    }
};


}  // namespace mir::caching
