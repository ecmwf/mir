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


#ifndef mir_action_transform_TransInitor_h
#define mir_action_transform_TransInitor_h


namespace mir {
namespace action {
namespace transform {


class TransInitor {
public:
    static const TransInitor& instance();
private:
    TransInitor();
    ~TransInitor();
};


}  // namespace transform
}  // namespace action
}  // namespace mir


#endif
