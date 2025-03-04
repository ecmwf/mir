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

import json
import os

# To run on lxb15
import gribapi

params = {}
SCORES = {}

SCORE = {"od": 10000, "e2": 1000, "e4": 100, "ei": 150, "er": 50, "rd": 10}

names = "shortName,type,stream,paramId,gridType,name,units".split(",")


def dump():
    out = {}
    for k, v in params.items():
        a = out[k] = {}
        for p, q in v.items():
            a[p] = ", ".join(sorted(q))

    with open("/perm/ma/mab/gribs/params.json", "w") as g:
        g.write(json.dumps(out, indent=4))


for root, dirs, files in os.walk("/gpfs/lxab/marsdev/mars_grib2/workdir/class"):

    for name in files:

        if not name.startswith("data."):
            continue

        path = os.path.join(root, name)
        if "/ai/" in path:
            continue

        print(path)
        with open(path) as f:
            while True:
                h = gribapi.grib_new_from_file(f)
                if h is None:
                    break

                id = "%s-%s" % (
                    gribapi.grib_get_string(h, "paramId"),
                    gribapi.grib_get_string(h, "gridType"),
                )
                cl = "%s" % gribapi.grib_get_string(h, "class")
                if id not in params or SCORE.get(cl, 0) > SCORES.get(id, 0):
                    print("New ID", id, gribapi.grib_get_string(h, "name"))
                    with open("/perm/ma/mab/gribs/%s.grib" % (id,), "w") as g:
                        g.write(gribapi.grib_get_message(h))
                    i = gribapi.grib_keys_iterator_new(h, "mars")
                    r = []
                    while True:
                        j = gribapi.grib_keys_iterator_next(i)
                        if not j:
                            break
                        n = gribapi.grib_keys_iterator_get_name(j)
                        r.append("%s=%s" % (n, gribapi.grib_get_string(h, n)))
                    gribapi.grib_keys_iterator_delete(h, i)

                    params.setdefault(id, {})
                    params[id]["mars"] = set([", ".join(r)])
                    SCORES[id] = SCORE.get(cl, 0)

                entry = params[id]

                for n in names:
                    entry.setdefault(n, set())
                    v = "?"
                    try:
                        v = "%s" % gribapi.grib_get_string(h, n)
                        if "unknown" in v:
                            print("unknown", n, "in", path)
                    except Exception:
                        v = "missing"

                    entry[n].add(v.strip())

                gribapi.grib_release(h)

    dump()

dump()
