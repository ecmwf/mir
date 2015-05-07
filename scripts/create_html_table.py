#!/usr/bin/env python
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

print """
<table>
<tbody>
"""

print "<tr>"
for n in names:
    print "<th>%s</th>" % (n[0],)

print "</tr>"

for k in sorted(params.keys(), key=lambda a: params[a]["name"]):
    p = params[k]
    print "<tr>"
    for n in names:
        v = p.get(n[1], '')
        print "<td>%s</td>" % (v),
    print
    print "</tr>"

print """
</tbody>
</table>

"""
