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


#ifndef mir_caching_matrix_SharedMemoryLoader_h
#define mir_caching_matrix_SharedMemoryLoader_h

#include "mir/caching/matrix/MatrixLoader.h"


namespace mir {
namespace caching {
namespace matrix {


class SharedMemoryLoader : public MatrixLoader {
public:
    SharedMemoryLoader(const std::string& name, const eckit::PathName&);

    virtual ~SharedMemoryLoader() override;

    static void loadSharedMemory(const eckit::PathName&);
    static void unloadSharedMemory(const eckit::PathName&);

protected:
    virtual void print(std::ostream&) const override;

private:
    virtual const void* address() const override;
    virtual size_t size() const override;
    virtual bool inSharedMemory() const override;

    void* address_;
    size_t size_;
    bool unload_;
};


}  // namespace matrix
}  // namespace caching
}  // namespace mir


#endif
