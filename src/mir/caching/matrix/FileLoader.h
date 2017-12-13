/*
 * (C) Copyright 1996-2015 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Baudouin Raoult
/// @author Pedro Maciel
/// @author Tiago Quintino
///
/// @date Oct 2016

#ifndef mir_caching_FileLoader_H
#define mir_caching_FileLoader_H

#include "eckit/memory/MemoryBuffer.h"
#include "mir/caching/matrix/MatrixLoader.h"

namespace mir {
namespace caching {
namespace matrix {


//----------------------------------------------------------------------------------------------------------------------

class FileLoader : public MatrixLoader {
public:
    FileLoader(const std::string& name, const eckit::PathName& path);

    ~FileLoader();

    static bool shared();

protected:
    virtual void print(std::ostream&) const;

private:
    eckit::MemoryBuffer buffer_;

    virtual const void* address() const;
    virtual size_t size() const;
    virtual bool inSharedMemory() const;
};


//----------------------------------------------------------------------------------------------------------------------


} // namespace matrix
} // namespace caching
} // namespace mir

#endif
