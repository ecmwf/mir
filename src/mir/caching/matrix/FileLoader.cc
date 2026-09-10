// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


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
