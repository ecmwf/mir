// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#include "mir/caching/matrix/MatrixLoader.h"


namespace mir::caching::matrix {


class SharedMemoryLoader : public MatrixLoader {
public:
    SharedMemoryLoader(const std::string& name, const eckit::PathName&);

    ~SharedMemoryLoader() override;

    static void loadSharedMemory(const eckit::PathName&);
    static void unloadSharedMemory(const eckit::PathName&);

protected:
    void print(std::ostream&) const override;

private:
    const void* address() const override;
    size_t size() const override;
    bool inSharedMemory() const override;

    void* address_;
    size_t size_;
    bool unload_;
};


}  // namespace mir::caching::matrix
