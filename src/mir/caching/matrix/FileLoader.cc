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
/// @author Pedro Maciel
/// @author Tiago Quintino
///
/// @date Oct 2016

#include "mir/caching/matrix/FileLoader.h"

#include <fcntl.h>
#include <iostream>
#include <sys/mman.h>

#include "eckit/io/StdFile.h"
#include "eckit/log/Bytes.h"
#include "eckit/log/Timer.h"

#include "mir/config/LibMir.h"
#include "mir/method/WeightMatrix.h"

using mir::method::WeightMatrix;

namespace mir {
namespace caching {
namespace matrix {


//----------------------------------------------------------------------------------------------------------------------


FileLoader::FileLoader(const std::string& name, const eckit::PathName& path) :
    MatrixLoader(name, path),
    buffer_(path.size()) {

    // Note: buffer size is based on file.size() -- which is assumed to be bigger than the memory footprint

    eckit::Log::debug<LibMir>() << "Loading interpolation matrix from " << path << std::endl;

    WeightMatrix w(path);

    w.dump(buffer_);
}

FileLoader::~FileLoader() {}

void FileLoader::print(std::ostream& out) const {
    out << "FileLoader[path=" << path_ << ",size=" << eckit::Bytes(buffer_.size()) << "]";
}

const void* FileLoader::address() const {
    return buffer_;
}

size_t FileLoader::size() const {
    return buffer_.size();
}

bool FileLoader::inSharedMemory() const {
    return false;
}

namespace {
static MatrixLoaderBuilder<FileLoader> loader("file-io");
}


//----------------------------------------------------------------------------------------------------------------------



} // namespace matrix
} // namespace caching
} // namespace mir
