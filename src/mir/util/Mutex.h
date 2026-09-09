// SPDX-FileCopyrightText: 1996- European Centre for Medium-Range Weather Forecasts (ECMWF)
// SPDX-License-Identifier: Apache-2.0


#pragma once

#define mir_ECKIT_THREADS

#if defined(mir_ECKIT_THREADS)
#include "eckit/thread/AutoLock.h"
#include "eckit/thread/Mutex.h"
#else
#include <mutex>
#endif


namespace mir::util {


#if defined(mir_ECKIT_THREADS)


using recursive_mutex = eckit::Mutex;

template <typename T>
using lock_guard = typename eckit::AutoLock<T>;

struct once_flag {
    pthread_once_t once_ = PTHREAD_ONCE_INIT;
};

template <class Callable>
inline void call_once(once_flag& flag, Callable&& fun) {
    pthread_once(&(flag.once_), fun);
}


#else


using std::call_once;
using std::lock_guard;
using std::once_flag;
using std::recursive_mutex;


#endif


}  // namespace mir::util
