/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#include "ThreadSafeMap.h"
#include "LegendrePolynomials.h"
#include <cassert>

// Internal class to assist in locking access
// within the current scope (typically within functions)
//
class ScopeLock {
public:

    inline ScopeLock(pthread_mutex_t& mutex) : mutex_(mutex) 
    { 
        pthread_mutex_lock(&mutex_);
    }
    inline ~ScopeLock() 
    {
        pthread_mutex_unlock(&mutex_);
    }

private:
    pthread_mutex_t& mutex_;
};


template <class T> 
ThreadSafeMap<T>::ThreadSafeMap(unsigned int maxLength/* has default */) 
: maxLength_(maxLength)
{
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    //pthread_mutexattr_settype(&attr,PTHREAD_MUTEX_RECURSIVE_NP);

    pthread_mutex_init(&mutex_,&attr);
    pthread_mutexattr_destroy(&attr);
}

template <class T> 
ThreadSafeMap<T>::~ThreadSafeMap() 
{
    {
        ScopeLock l(mutex_);

        // remove all items from the collection. Deallocation is done within the
        // pointer class when no pointers exist to the memory
        //
        items_.erase( items_.begin(), items_.end() );
    }
    pthread_mutex_destroy(&mutex_);
}

template <class T>
ref_counted_ptr<const T> ThreadSafeMap<T>::getItem(const std::string& name) const
{
    ScopeLock l(mutex_);
    
    typename ThreadSafeMap<T>::map::const_iterator it = items_.find(name);
    if (it != items_.end())
    {
        return it->second;
    }

    // return a blank if not found. User should test validity before using.
    return ref_counted_ptr<const T>();
}

template <class T>
ref_counted_ptr<const T> ThreadSafeMap<T>::addItem(const std::string& name, const T* item)
{
    ScopeLock l(mutex_);

    typename ThreadSafeMap<T>::map::iterator it = items_.find(name);
    if (it != items_.end())
    {
        // It is already in the collection so return ref to existing item
        // Delete passed object.
        delete item;
        return it->second;
    }
    
    // now check we will not breach max size
    it = items_.begin();

    while (items_.size() >= maxLength_ && it != items_.end())
    {
        // we would rather get rid of one not currently used
        // but if we're on the final iteration we delete the last one
        // NB any smart pointers that exist will still be valid
        if (it->second.canDelete() || std::distance(it, items_.end()) == 1)
            items_.erase(it);

        it++;
    }

    assert(items_.size() < maxLength_);

    ref_counted_ptr<const T> newItem(item);
    items_.insert(make_pair(name, newItem));

    // return pointer to added item so we know it will not be deleted
    // before we can use it
    return newItem;
}

template <class T>
bool ThreadSafeMap<T>::removeItem(const std::string& name)
{
    ScopeLock l(mutex_);
    
    typename ThreadSafeMap<T>::map::iterator it = items_.find(name);
    if (it == items_.end())
        return false;

    items_.erase(it);
    return true;
}

// explicitly declare typed versions that we will use to avoid putting all in
// the header
//
template ThreadSafeMap<LegendrePolynomials>::ThreadSafeMap(unsigned int);
template ThreadSafeMap<LegendrePolynomials>::~ThreadSafeMap();
template ref_counted_ptr<const LegendrePolynomials> ThreadSafeMap<LegendrePolynomials>::getItem(const std::string&) const;
template ref_counted_ptr<const LegendrePolynomials> ThreadSafeMap<LegendrePolynomials>::addItem(const std::string&, const LegendrePolynomials*);
template bool ThreadSafeMap<LegendrePolynomials>::removeItem(const std::string&);

// for definitions cache
template ThreadSafeMap< vector<string> >::ThreadSafeMap(unsigned int);
template ThreadSafeMap< vector<string> >::~ThreadSafeMap();
template ref_counted_ptr< const vector<string> > ThreadSafeMap< vector<string> >::getItem(const std::string&) const;
template ref_counted_ptr< const vector<string> > ThreadSafeMap< vector<string> >::addItem(const std::string&, const vector<string>*);
template bool ThreadSafeMap< vector<string> >::removeItem(const std::string&);

// for lsm cache (or vector<double> generally)
template ThreadSafeMap< vector<double> >::ThreadSafeMap(unsigned int);
template ThreadSafeMap< vector<double> >::~ThreadSafeMap();
template ref_counted_ptr< const vector<double> > ThreadSafeMap< vector<double> >::getItem(const std::string&) const;
template ref_counted_ptr< const vector<double> > ThreadSafeMap< vector<double> >::addItem(const std::string&, const vector<double>*);
template bool ThreadSafeMap< vector<double> >::removeItem(const std::string&);
     




