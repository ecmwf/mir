/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/
/**************************************
 *  Sinisa Curic
 **************************************/

#include "LegendrePolynomialsSharedMemory.h"

#ifndef Exception_H
#include "Exception.h"
#endif

#ifndef Grid_H
#include "Grid.h"
#endif

#include <sstream>
#include <sys/mman.h>

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <errno.h>

#include <sys/stat.h>
#include <fcntl.h>
#include <sys/time.h>

#include <sys/sem.h>
#include <cstring>

/* buffer for loading from file */
#define LOAD_BUFFER_SIZE 10485760 
#define MAGIC_VALUE      1234567890

struct sembuf _lock[] = {
    { 0, 0,  SEM_UNDO }, /* test */
    { 0, 1,  SEM_UNDO }, /* lock */
};

struct sembuf _unlock[] = {
    { 0, -1, SEM_UNDO }, /* ulck */
};

#define INFO_PATH 1024

struct info {
    int ready;
    int magic;
    char path[INFO_PATH];
};

LegendrePolynomialsSharedMemory::LegendrePolynomialsSharedMemory(int truncation, const Grid& grid) : 
	LegendrePolynomialsRead(truncation,grid)
{
	string file = constructCoefficientsFilename();

    char path[1024];
    struct  stat64 stat1;
    struct  stat64 stat2;
    key_t key;
    struct timeval start, end, diff;
    int loadfile = 1;
    int pageSize = getpagesize();

    if(pageSize < 0)
	  	throw BadParameter("LegendrePolynomialsSharedMemory::shareFile getpagesize fail: " + file);

    if(file.size() > INFO_PATH)
	  	throw WrongValue("LegendrePolynomialsSharedMemory::shareFile path too long: ", file.size());

	if(realpath(file.c_str(),path) == 0)
	  	throw BadParameter("LegendrePolynomialsSharedMemory::shareFile realpath: " + file);
		
    if(sizeof(stat1.st_size) < 8)
	  	throw WrongValue("LegendrePolynomialsSharedMemory::shareFile stat.st_size is to small for 64bits files: ", stat1.st_size);


    key = ftok(path,1);

    if(key ==  (key_t)-1)
	  	throw BadParameter("LegendrePolynomialsSharedMemory::shareFile ftok: " + file);


    sem_ = semget(key,1,IPC_CREAT|0600);

    if(sem_ < 0)
	  	throw BadParameter("LegendrePolynomialsSharedMemory::shareFile semget: " + file);

	if(SHARED_DEBUG) 
		gettimeofday( &start, NULL );

    if(semop(sem_,_lock, 2 ) < 0)
	  	throw BadParameter("LegendrePolynomialsSharedMemory::shareFile semop lock: " + file);

    if (SHARED_DEBUG) {
        gettimeofday( &end, NULL );

        diff.tv_sec  = end.tv_sec  - start.tv_sec;
        diff.tv_usec = end.tv_usec - start.tv_usec;

        if (diff.tv_usec < 0)
        {
            diff.tv_sec--;
            diff.tv_sec--;
            diff.tv_usec += 1000000;
        }
        double time = (double)diff.tv_sec + ((double)diff.tv_usec / 1000000.);

        cout<< "LegendrePolynomialsSharedMemory::shareFile:semop:lock wait " << time << " secs" << endl;
    }

    openCoefficientsFile(O_LARGEFILE);

    if(stat64(path,&stat1)) 
	  	throw BadParameter("LegendrePolynomialsSharedMemory::shareFile stat64 fail: " + file);

    size_t baseSizeRequired = ((stat1.st_size + pageSize-1)/pageSize)*pageSize + sizeof(struct info);
    
    size_t sizeRequired = pageSize * (1 + baseSizeRequired/pageSize);

#ifdef linux
    // See how much we can allocate per segment
    // default to the commonly-supported 32MB
    unsigned long shmmax = 32 * 1024 * 1024;

    struct shminfo info;
    if (shmctl(0, IPC_INFO, (struct shmid_ds *)(void *)&info) != -1)
        shmmax = info.shmmax;

    //int nsegs = 1 + (sizeRequired / shmmax);
    int segshift = (sizeRequired / shmmax);
#else 
    // on AIX we can assume we can allocate entire segment
    unsigned long shmmax = sizeRequired;
    int nsegs = 1;
    int segshift = 0;
#endif

    // keep count of amount mapped incrementally
    size_t sizeMapped = 0;

    char* addr;
    struct info* nfo;

    void* next_seg_loc = NULL;
    int counter = 0;
    segments_.clear();

    while (sizeMapped < sizeRequired)
    {
#ifdef linux
        key_t seg_key = ftok(path, counter);
#else
        key_t seg_key = key;
#endif

        size_t sizeSegment = min(sizeRequired-sizeMapped, shmmax);

        // try to connect to existing segment
        int shmid = shmget(seg_key, sizeSegment, 0600);

        if(shmid  < 0)
        {
            // if it doesn't exist, try allocating
            if (ENOENT == errno)
                shmid = shmget(seg_key, sizeSegment, IPC_CREAT|0600);

            if (shmid < 0)
                throw BadParameter("LegendrePolynomialsSharedMemory::shareFile shmget fail (memory error): " + file);

#ifdef SHM_PAGESIZE
            // Use 64K pages to back the shared memory region 
            // NB This can only be set immediately after creation of a segment 
            // and before it has ever been attached to a process
            // Hence don't use on segments we didn't create in this process
            size_t shm_size;
            struct shmid_ds shm_buf = { 0 };
            psize_t psize_64k = 64 * 1024;

            shm_buf.shm_pagesize = psize_64k;
            if (shmctl(shmid, SHM_PAGESIZE, &shm_buf))
                throw BadParameter("LegendrePolynomialsSharedMemory::shareFile shmctl(SHM_PAGESIZE) fail: " + file);
#endif
        }

        // Get the size of the seg you allocated from shmctl and check it is
        // correct
        struct shmid_ds shm_data;
        if (shmctl(shmid, IPC_STAT, &shm_data) == -1)
            throw BadParameter("LegendrePolynomialsSharedMemory::shareFile shmctl fail (memory error): " + file);

        if (shm_data.shm_segsz != sizeSegment)
            throw BadParameter("LegendrePolynomialsSharedMemory::shareFile shmget error (memory error): " + file);

        sizeMapped += sizeSegment;

        /* attach shared memory */
        void* ptr = shmat( shmid, next_seg_loc, 0 );
        if (ptr == (void*)-1) 
        {
            throw BadParameter("LegendrePolynomialsSharedMemory::shareFile shmat fail: " + file);
        }

        // use the location of where that first segment was allocated to work
        // out where to move it to such that all segments are mapped
        // to available areas (i.e. at lower memory addresses)
        //
        if (0 == counter )
        {
            // we have requested automatic map address the first time
            //
            if (segshift > 0)
            {
                // if we need to detatch and move to lower addresses, do it
                // here
                shmdt( ptr );
                
                next_seg_loc = (void*)((long)ptr - (segshift * shmmax));

                // reattach to this lower position
                ptr = shmat( shmid, next_seg_loc, 0 );
                if (ptr == (void*)-1) 
                    throw BadParameter("LegendrePolynomialsSharedMemory::shareFile shmat fail: " + file);

            }

            // first time through, store the base pointer to the resultant
            // contiguous memory block we will form from the multiple segments
            ptr_ = ptr;

        } 
        if (counter == segshift)
        {
            // if the last time through, we need to write the below to the end
            // of the entire memory block
            //
            addr = (char*)ptr_;
            nfo = (struct info*)(addr + (((stat1.st_size + pageSize - 1) / pageSize) * pageSize));

            if(nfo->ready) { 
                loadfile = 0;
                if(nfo->magic != MAGIC_VALUE) {
                    throw WrongValue("LegendrePolynomialsSharedMemory::shareFile bad magic: ", nfo->magic);
                }

                if(strcmp(nfo->path,path) != 0)
                    throw BadParameter("LegendrePolynomialsSharedMemory::shareFile invalid path: ");
            }
            
        }

        // store seg pointer so we can detach in dtor
        segments_.push_back(ptr);
        
        // work out where we put the next one
        next_seg_loc = (void*)((long)ptr + sizeSegment);
        counter++;
    }

    
    // here we read info from the loaded memory to see whether we should read
    // from the file into the memory
    addr = (char*)ptr_;
    nfo = (struct info*)(addr + (((stat1.st_size + pageSize - 1) / pageSize) * pageSize));

    if(nfo->ready) 
    { 
        loadfile = 0;
        if(nfo->magic != MAGIC_VALUE) {
            throw WrongValue("LegendrePolynomialsSharedMemory::shareFile bad magic: ", nfo->magic);
        }

        if(strcmp(nfo->path,path) != 0)
            throw BadParameter("LegendrePolynomialsSharedMemory::shareFile invalid path: ");
    }
    
    if(loadfile) {

        stat2.st_size =  stat1.st_size;

        if (SHARED_DEBUG) 
			gettimeofday( &start, NULL );

        while(stat1.st_size > 0) 
        {
            ssize_t len = stat1.st_size > LOAD_BUFFER_SIZE ? LOAD_BUFFER_SIZE : stat1.st_size;
            if(read(fd_, addr, len) != len) {
	  			throw ReadError("LegendrePolynomialsSharedMemory::shareFile: " + file);
            }
            stat1.st_size -= len;
            addr      += len;
        }

        if (SHARED_DEBUG)
    	{
            gettimeofday( &end, NULL );

            diff.tv_sec  = end.tv_sec  - start.tv_sec;
            diff.tv_usec = end.tv_usec - start.tv_usec;

            if (diff.tv_usec < 0)
            {
                diff.tv_sec--;
                diff.tv_usec += 1000000;
            }
            double time = (double)diff.tv_sec + ((double)diff.tv_usec / 1000000.);

        	cout<< "LegendrePolynomialsSharedMemory::shareFile:read " << stat2.st_size << " bytes in " << time << " secs" << endl;
        }

        nfo->magic = MAGIC_VALUE;
        strcpy(nfo->path,path);
        nfo->ready = 1;
    }
    else
    {
        if (SHARED_DEBUG)
        	cout<< "LegendrePolynomialsSharedMemory::shareFile:read file already loaded" << endl;
    }

    closeFile();

    next_ = (double*)ptr_;

    if(semop(sem_,_unlock,1) < 0){
	  	throw BadParameter("LegendrePolynomialsSharedMemory::shareFile semop unlock fail");
    }
}
	

LegendrePolynomialsSharedMemory::~LegendrePolynomialsSharedMemory()
{
    if (DEBUG)
        cout << "LegendrePolynomialsSharedMemory::~LegendrePolynomialsSharedMemory destructor called" << endl;


    closeFile();
// Detach here...

    /*
	if(ptr_){
    	shmdt(ptr_);
        ptr_ = NULL;
	}
    */
    ptr_ = NULL;
    next_ = NULL;
    detachSegments();

    //struct sembuf data;
    int count = semctl(sem_, 0, GETVAL);
    if( count > 0 && semop(sem_,_unlock,1) < 0){
	  	throw BadParameter("LegendrePolynomialsSharedMemory::~LegendrePolynomialsSharedMemory semop unlock fail");
    }
}

void LegendrePolynomialsSharedMemory::detachSegments()
{
    for (unsigned int i = 0; i < segments_.size(); i++)
    {
        shmdt(segments_[i]);
    }
    segments_.clear();
}

const double* LegendrePolynomialsSharedMemory::getOneLatitude(double lat, int rowOffset) const
{
//	memcpy(polynoms, next_ + latLength_ * rowOffset, latSize_);
//	return polynoms;
	return 0;
}

void LegendrePolynomialsSharedMemory::print(ostream& out) const
{
	LegendrePolynomialsRead::print(out);
	out << "Shared Memory";
}
