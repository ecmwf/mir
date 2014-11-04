# Copyright (C) 2009 ECMWF

import os

# We solve to proxy problem in one place
if "http_proxy" in os.environ:
    del os.environ["http_proxy"]

site_dir = os.path.abspath(os.path.dirname(__file__))

DEBUG = True
TEMPLATE_DEBUG = DEBUG

# speeds things along
TEMPLATE_DEBUG = False

ADMINS = (
        #("Peter Bispham", "peter.bispham@ecmwf.int"),
)

MANAGERS = ADMINS

# no database support required
DATABASE_ENGINE = ""
DATABASE_NAME = ""
DATABASE_USER = ""
DATABASE_PASSWORD = ""
DATABASE_HOST = ""
DATABASE_PORT = ""

MEDIA_ROOT = os.path.join(site_dir, "static")
MEDIA_URL = "/static/"
ADMIN_MEDIA_PREFIX = "/media/"

STATIC_ROOT = os.path.join(site_dir, "static")
STATIC_URL= "/output/"

SESSION_COOKIE_NAME = "mir-development"
SESSION_ENGINE = "django.contrib.sessions.backends.file" 

# default results folder
RESULTS_FOLDER="/tmp/matq/testcases/mir/suite/output"
#RESULTS_FOLDER="/scratch/ma/maf/scratchdir/ecRegrid/bigtest/output"
SHOW_JIRA_SECTION=False

# path to magjson (including trailing slash)
MAGJSON_PATH="/usr/local/apps/Magics/current/bin/"

SESSION_COOKIE_DOMAIN = ".ecmwf.int"

TIME_ZONE = "UTC"

LANGUAGE_CODE = "en-us"

SITE_ID = 2

USE_I18N = False

SECRET_KEY = "-q=z7j4i^o$^gbm6&=*kq%opjl@nw(($bqh7zok6257sr5pe3o"

TEMPLATE_LOADERS = (
    'django.template.loaders.filesystem.Loader',
    'django.template.loaders.app_directories.Loader', 
)

MIDDLEWARE_CLASSES = (
    "django.middleware.common.CommonMiddleware",
    "django.contrib.sessions.middleware.SessionMiddleware",
)

ROOT_URLCONF = "urls"

# the place where templates will resider
TEMPLATE_DIRS = (
    os.path.join(site_dir, "templates")
)

INSTALLED_APPS = (
    "django.contrib.auth",
    "django.contrib.admin",
    "django.contrib.admindocs",
    "django.contrib.contenttypes",
    "django.contrib.sessions",
    "django.contrib.humanize",
    "viewer",
)
