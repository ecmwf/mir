#!/usr/bin/env python2
from mpl_toolkits.basemap import Basemap
import matplotlib.pyplot as plt
import sys

lats = []
lons = []
with open(sys.argv[1]) as f:
    for line in f:
        x = line.split(line, ' ')
        lats.append(float(x[0]))
        lons.append(float(x[1]))


north = max(lats)
south = min(lats)
east = max(lons)
west = min(lons)

map = Basemap(projection='merc',
              lat_0=(north + south) / 2.0,
              lon_0=(west + east) / 2.0,
              # resolution = 'h', area_thresh = 0.1,
              llcrnrlon=west, llcrnrlat=south,
              urcrnrlon=east, urcrnrlat=north)

map.drawcoastlines()
map.drawcountries()
map.fillcontinents(color='coral')
map.drawmapboundary()

x, y = map(lons, lats)
map.plot(x, y, 'bo', markersize=2)

plt.show()
