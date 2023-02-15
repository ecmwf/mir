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


#include "mir/caching/matrix/FileLoader.h"

#include <ostream>

#include "mir/method/WeightMatrix.h"
#include "mir/util/Log.h"
#include "mir/util/Types.h"


namespace mir::caching::matrix {


FileLoader::FileLoader(const std::string& name, const eckit::PathName& path) :
    MatrixLoader(name, path), buffer_(size_t(path.size())) {

    Log::debug() << "Loading matrix from " << path << std::endl;

    // Note: buffer size is based on file.size() -- which is assumed to be bigger than the memory footprint
    method::WeightMatrix w(path);
    w.dump(buffer_);
}

FileLoader::~FileLoader() = default;

void FileLoader::print(std::ostream& out) const {
    out << "FileLoader[path=" << path_ << ",size=" << Log::Bytes(buffer_.size()) << "]";
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


static const MatrixLoaderBuilder<FileLoader> loader("file-io");


}  // namespace mir::caching::matrix
