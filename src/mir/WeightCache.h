/*
 * (C) Copyright 1996-2014 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Peter Bispham
/// @author Tiago Quintino
/// @date Oct 2013

#ifndef mir_WeightCache_H
#define mir_WeightCache_H

#include "eckit/memory/NonCopyable.h"
#include "eckit/thread/Mutex.h"
#include <Eigen/Sparse>

//-----------------------------------------------------------------------------

namespace mir {

//-----------------------------------------------------------------------------

class WeightCache : private eckit::NonCopyable {

public: // methods

    WeightCache();

    virtual ~WeightCache();

    // fills passed matrix with cached values and returns true if found cache
    bool get(const std::string& key, Eigen::SparseMatrix<double>& W) const;
    bool add(const std::string& key, Eigen::SparseMatrix<double>& W ) const;

    std::string filename(const std::string& key) const;

protected:

    mutable eckit::Mutex mutex_;

};

} // namespace mir

#endif
