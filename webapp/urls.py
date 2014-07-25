# Copyright (C) 2012 ECMWF

from django.conf import settings
from django.contrib import admin
from django.conf.urls.defaults import *


admin.autodiscover()

urlpatterns = patterns("",
    (r"^viewer/", include("viewer.urls")),
    (r"^admin/doc/", include("django.contrib.admindocs.urls")),
    (r"^admin/", include(admin.site.urls)),
)

if settings.DEBUG:
    path = settings.MEDIA_URL.strip("/")
    urlpatterns += patterns("",
                            (r"^static/(?P<path>.*)$", 'django.views.static.serve',
                             {'document_root': settings.MEDIA_ROOT,
                              "show_indexes": True}))
