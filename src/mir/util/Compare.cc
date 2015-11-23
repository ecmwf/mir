/*
 * (C) Copyright 1996-2015 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Pedro Maciel
/// @date May 2015


#include "mir/util/Compare.h"


namespace mir {
namespace util {
namespace compare {


const is_approx_equal_fn< double > is_approx_zero (0.);
const is_approx_equal_fn< double > is_approx_one  (1.);


}  // namespace compare
}  // namespace util
}  // namespace mir

