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
/// @date Aug 2019


#ifndef MIREstimation_H
#define MIREstimation_H



namespace mir {

namespace api {


class MIREstimation {
public:

    virtual void numberOfGridPoints(size_t count) = 0;
    virtual void accuracy(size_t count) = 0;
    virtual void edition(size_t count) = 0;
    virtual void packing(const std::string& packing) = 0;
    virtual void representation(const std::string& representation) = 0;
    virtual void truncation(size_t count) = 0;

};


}  // namespace api
}  // namespace mir
#endif

