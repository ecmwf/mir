/*
 * (C) Copyright 1996-2015 ECMWF.
 *
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
 * In applying this licence, ECMWF does not waive the privileges and immunities
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

/// @author Baudouin Raoult
/// @author Peter Bispham
/// @author Tiago Quintino
/// @author Pedro Maciel
///
/// @date May 2015


#include "mir/method/MatrixCacheCreator.h"
#include "mir/method/MethodWeighted.h"

#include <unistd.h>

namespace mir {
namespace method {


MatrixCacheCreator::MatrixCacheCreator(const MethodWeighted& owner,
                                       context::Context& ctx,
                                       const repres::Representation& in,
                                       const repres::Representation& out,
                                       const lsm::LandSeaMasks& masks):
    owner_(owner),
    ctx_(ctx),
    in_(in),
    out_(out),
    masks_(masks) {

}

void MatrixCacheCreator::create(const eckit::PathName& path, WeightMatrix& W, bool& saved) {

    pid_t pid = ::fork();
    switch (pid) {

    case 0:
        // child
        eckit::Log::info() << "MatrixCacheCreator::create running in sub-process " << ::getpid() << std::endl;

        try {
            owner_.createMatrix(ctx_, in_, out_, W, masks_);
            W.save(path);
            ::_exit(0);
        }
        catch (std::exception& e) {
            eckit::Log::error() << "MatrixCacheCreator::create failed " << e.what() << std::endl;
        }
        ::_exit(1);
        break;

    case -1:
        // error
        eckit::Log::error() << "MatrixCacheCreator::create failed to fork(): "
                            << eckit::Log::syserr
                            << std::endl;
        owner_.createMatrix(ctx_, in_, out_, W, masks_);
        return;
        break;

    }

    // Parent

    eckit::Log::info() << "MatrixCacheCreator::create wait for " << pid << std::endl;
    int code = 0;
    SYSCALL(::waitpid(pid, &code, 0));
    saved = true;
    eckit::Log::info() << "MatrixCacheCreator::create " << pid << " finished with code " << code << std::endl;
    ASSERT(code == 0);

}

}  // namespace method
}  // namespace mir

