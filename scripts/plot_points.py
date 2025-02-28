#!/usr/bin/env python3
#
# (C) Copyright 1996- ECMWF.
#
# This software is licensed under the terms of the Apache Licence Version 2.0
# which can be obtained at http://www.apache.org/licenses/LICENSE-2.0.
#
# In applying this licence, ECMWF does not waive the privileges and immunities
# granted to it by virtue of its status as an intergovernmental organisation nor
# does it submit to any jurisdiction.

import sys

import matplotlib.pyplot as plt
from mpl_toolkits.basemap import Basemap

lats = []
lons = []
with open(sys.argv[1]) as f:
    for line in f:
        x = line.split(" ")
        lats.append(float(x[0]))
        lons.append(float(x[1]))


north = max(lats)
south = min(lats)
east = max(lons)
west = min(lons)

print(north, west, south, east)

map = Basemap(projection="cyl", lat_0=(north + south) / 2.0, lon_0=(west + east) / 2.0)
# # resolution = 'h', area_thresh = 0.1,
# llcrnrlon=west, llcrnrlat=south,
# urcrnrlon=east, urcrnrlat=north)

map.drawcoastlines()
map.drawcountries()
map.fillcontinents(color="coral")
map.drawmapboundary()

x, y = map(lons, lats)
map.plot(x, y, "bo", markersize=0.1)

plt.show()
