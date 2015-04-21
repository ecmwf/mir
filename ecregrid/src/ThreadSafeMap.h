/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/
#ifndef ThreadSafeMap_H
#define ThreadSafeMap_H


#include <map>
#include <string>
#include <pthread.h>
#include "ref_counted_ptr.h"

template <class T> class ThreadSafeMap {
    // Manages a set of objects
    // in a thread-safe manner.

  public:

    ThreadSafeMap(unsigned int maxLength=5);
    virtual ~ThreadSafeMap();

    // accessors
    // return const pointers so callers cannot modify cached object
    ref_counted_ptr<const T> getItem(const std::string& name) const;
    ref_counted_ptr<const T> addItem(const std::string& name, const T* item);
    bool removeItem(const std::string& name);

  private:

    // attributes
    typedef std::map< std::string, ref_counted_ptr<const T> > map;
    map items_;

    mutable pthread_mutex_t mutex_;

    const unsigned int maxLength_;

};


#endif // ThreadSafeMap_H





