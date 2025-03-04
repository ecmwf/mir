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


#pragma once

#include <cstddef>
#include <string>


namespace mir::api {


class MIREstimation {
public:
    MIREstimation() = default;

    MIREstimation(const MIREstimation&) = delete;
    MIREstimation(MIREstimation&&)      = delete;

    virtual ~MIREstimation() = default;

    void operator=(const MIREstimation&) = delete;
    void operator=(MIREstimation&&)      = delete;

    virtual void numberOfGridPoints(size_t count)                  = 0;
    virtual void missingValues(size_t count)                       = 0;
    virtual void pl(size_t count)                                  = 0;
    virtual void accuracy(size_t count)                            = 0;
    virtual void edition(size_t count)                             = 0;
    virtual void packing(const std::string& packing)               = 0;
    virtual void representation(const std::string& representation) = 0;
    virtual void truncation(size_t count)                          = 0;
    virtual void sameAsInput()                                     = 0;
};


}  // namespace mir::api
