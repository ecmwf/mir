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

# To run on lxb15
import json

names = [
    ("Name", "name",),
    ("Abbr.", "shortName",),
    ("Type", "type",),
    ("Stream", "stream",),
    ("ID", "paramId",),
    ("Grid", "gridType",),
    ("Units", "units",),
    ("Method", None),
    ("Comment", None),
    ("Who", None),
    ("Request", "mars",),
]

with open("/perm/ma/mab/gribs/params.json") as f:
    params = json.loads(f.read())

print(
    """
<table>
<tbody>
"""
)

print("<tr>")
for n in names:
    print("<th>%s</th>" % (n[0],))

print("</tr>")

for k in sorted(params.keys(), key=lambda a: params[a]["name"]):
    p = params[k]
    print("<tr>")
    for n in names:
        v = p.get(n[1], "")
        print("<td>%s</td>" % (v),)
    print()
    print("</tr>")

print(
    """
</tbody>
</table>

"""
)
