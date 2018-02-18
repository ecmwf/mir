/*
 * (C) Copyright 1996- ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @date Jan 2017


#include "mir/action/transform/TransInitor.h"

#include "mir/api/Atlas.h"


#include "eckit/exception/Exceptions.h"

namespace mir {
namespace action {
namespace transform {


const TransInitor& TransInitor::instance() {
    static TransInitor initor;
    return initor;
}


TransInitor::TransInitor() {
    trans_use_mpi(false); // So that even if MPI is enabled, we don't use it.
    trans_init();
}


TransInitor::~TransInitor() {
    trans_use_mpi(false); // So that even if MPI is enabled, we don't use it.
    trans_finalize();
}


}  // namespace transform
}  // namespace action
}  // namespace mir
