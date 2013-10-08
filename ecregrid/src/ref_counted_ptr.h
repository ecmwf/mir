/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#ifndef RefCountedPtr_H
#define RefCountedPtr_H

template <class T> class ref_counted_ptr
{
    // smart pointer to count references and delete when necessary.
    
public:

    ref_counted_ptr() : pData_(0), count_(new long(1)) 
    {
    
    }
    ref_counted_ptr(T* pData) : pData_(pData), count_(new long(1))
    {
    }

    ref_counted_ptr(const ref_counted_ptr& other) : pData_(other.pData_), count_(other.count_)
    {
        addRef();
    }

    virtual ~ref_counted_ptr()
    {
        dispose();
    }

    // accessors 
    T& operator*() const
    {
        return *pData_;
    }

    T* operator->() const
    {
        return pData_;
    }

    T* get() const
    {
        return pData_;
    }

    ref_counted_ptr<T>& operator=(const ref_counted_ptr<T>& other)
    {
        if (this != &other)
        {
            dispose();
            pData_ = other.pData_;
            count_ = other.count_;
            addRef();
        }
        return *this;

    };

    long count() const { return *count_; }
    bool canDelete() const { return count() == 1; }

private:

    void addRef()
    {
        ++*count_;
    }

    void dispose()
    {
        --*count_;
        if (*count_ == 0)
        {
            delete pData_;
            delete count_;
        }
    }

    T* pData_;
    long* count_;
};

#endif // RefCountedPtr_H
