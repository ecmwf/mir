/***************************** LICENSE START ***********************************

 Copyright 2012 ECMWF. This software is distributed under the terms
 of the Apache License version 2.0. In applying this license, ECMWF does not
 waive the privileges and immunities granted to it by virtue of its status as
 an Intergovernmental Organization or submit itself to any jurisdiction.

 ***************************** LICENSE END *************************************/

#ifndef Definitions_Cache_H
#define Definitions_Cache_H

#include "ThreadSafeMap.h"
#include "Exception.h"
#include <vector>
#include <string>
#include <fstream>

class DefinitionsCache {

  public:

    static ref_counted_ptr< const vector<string> > get(const string& name) {
        // Do we already have it?
        ref_counted_ptr< const vector<string> > config = configData.getItem(name);

        if (config.get())
            return config;

        // It isn't cached. try and open the file
        ifstream in(name.c_str());
        if (!in.is_open())
            throw CantOpenFile(name);

        // Read line-by-line from file
        vector<string>* newItem = new vector<string>();
        string buffer;
        while( getline(in, buffer) )
            newItem->push_back(buffer);
        in.close();

        // Add new item to the cache. Ref-counted pointer returned
        return configData.addItem(name, newItem);

    }

  protected:

    // the cache itself
    static ThreadSafeMap< vector<string> > configData;

};


#endif // Definitions_Cache_H
