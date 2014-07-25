# Copyright (C) 2009 ECMWF

from django.http import HttpResponse


def to_json(istr):
    """Encode passed string as json argument
    """
    return json.dumps(istr, separators=(",", ":"))

def as_json(func):
    """Encode the returned value of ``func`` as a JSON value, and wrap it in a
    Django HTTP response object.

    """
    def f(*args, **kwargs):
        rtn = func(*args, **kwargs)
        res = to_json(rtn)
        return HttpResponse(res, mimetype="application/json")
    return f
