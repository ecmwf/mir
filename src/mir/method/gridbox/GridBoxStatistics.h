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


#ifndef mir_method_gridbox_GridBoxStatistics_h
#define mir_method_gridbox_GridBoxStatistics_h

#include "mir/method/gridbox/GridBoxMethod.h"


namespace mir {
namespace method {
namespace gridbox {


struct GridBoxStatistics final : GridBoxMethod {
    explicit GridBoxStatistics(const param::MIRParametrisation&);
    GridBoxStatistics(const GridBoxStatistics&) = delete;
    void operator=(const GridBoxStatistics&) = delete;
};


}  // namespace gridbox
}  // namespace method
}  // namespace mir


#endif
