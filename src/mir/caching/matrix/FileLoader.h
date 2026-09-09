// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include "eckit/memory/MemoryBuffer.h"

#include "mir/caching/matrix/MatrixLoader.h"


namespace mir::caching::matrix {


class FileLoader : public MatrixLoader {
public:
    FileLoader(const std::string& name, const eckit::PathName&);

    ~FileLoader() override;

    static bool shared();

protected:
    void print(std::ostream&) const override;

private:
    const void* address() const override;
    size_t size() const override;
    bool inSharedMemory() const override;

    eckit::MemoryBuffer buffer_;
};


}  // namespace mir::caching::matrix
