// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#include <sys/wait.h>
#include <unistd.h>

#include "eckit/config/Resource.h"
#include "eckit/thread/AutoLock.h"

#include "mir/method/MatrixCacheCreator.h"
#include "mir/method/MethodWeighted.h"
#include "mir/util/Exceptions.h"
#include "mir/util/Log.h"


namespace mir::method {


MatrixCacheCreator::MatrixCacheCreator(const MethodWeighted& owner, context::Context& ctx,
                                       const repres::Representation& in, const repres::Representation& out,
                                       const lsm::LandSeaMasks& masks, const Cropping& cropping) :
    owner_(owner), ctx_(ctx), in_(in), out_(out), masks_(masks), cropping_(cropping) {}

void MatrixCacheCreator::create(const eckit::PathName& path, WeightMatrix& W, bool& saved) {

    static bool subProcess = eckit::Resource<bool>("$MATRIX_CACHE_CREATOR_FORK", false);

    if (!subProcess) {
        owner_.createMatrix(ctx_, in_, out_, W, masks_, cropping_);
        return;
    }

    eckit::CacheManagerFileFlock lockfile("/tmp/mir.fork.lock");
    eckit::AutoLock<eckit::CacheManagerFileFlock> lock(lockfile);


    pid_t pid = ::fork();
    switch (pid) {

        case 0:
            // child
            Log::info() << "MatrixCacheCreator::create running in sub-process " << ::getpid() << std::endl;

            try {
                owner_.createMatrix(ctx_, in_, out_, W, masks_, cropping_);
                W.save(path);
                ::_exit(0);
            }
            catch (std::exception& e) {
                Log::error() << "MatrixCacheCreator::create failed " << e.what() << std::endl;
            }
            ::_exit(1);
            // break;

        case -1:
            // error
            Log::error() << "MatrixCacheCreator::create failed to fork(): " << Log::syserr << std::endl;
            owner_.createMatrix(ctx_, in_, out_, W, masks_, cropping_);
            return;
            // break;
    }

    // Parent

    Log::info() << "MatrixCacheCreator::create wait for " << pid << std::endl;
    int code = 0;
    SYSCALL(::waitpid(pid, &code, 0));
    saved = true;
    Log::info() << "MatrixCacheCreator::create " << pid << " finished with code " << code << std::endl;
    ASSERT(code == 0);
}

}  // namespace mir::method
