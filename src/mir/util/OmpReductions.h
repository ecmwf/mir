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

#include <vector>
#include <algorithm>
#include <omp.h> 

#pragma omp declare reduction(vec_merge_sorted : std::vector<size_t> : \
    omp_out.insert(omp_out.end(), omp_in.begin(), omp_in.end()); \
    std::inplace_merge(omp_out.begin(), omp_out.begin() + omp_out.size() - omp_in.size(), omp_out.end())) \
    initializer(omp_priv = std::vector<size_t>()) 

