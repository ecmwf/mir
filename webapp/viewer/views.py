# Create your views here.
import os
import datetime
import pprint
import math
import time
import copy
import numpy
from types import *

from django.shortcuts import render_to_response
from django.template import RequestContext
from django.http import HttpResponse
from django.core.servers.basehttp import FileWrapper
#from django.contrib.sessions.backends.db import SessionStore

from django.conf import settings
from xml.dom.pulldom import *
from xml.dom.pulldom import *
from dateutil.relativedelta import relativedelta

from collections import OrderedDict

from PIL import Image

#from servicelib import encoding as json
import json
from gribapi import *

#from django_tools.middlewares import ThreadLocal

import uuid
from urlparse import urlparse

jira_api = "https://software.ecmwf.int/issues/rest/api/latest"
jira_stem = "http://software.ecmwf.int/issues/browse"


image_layout_options = [
                        "stacked", 
                        "side",  
                        "comparison"
                       ]

# the amount of scaling to be applied to the diff values in the grib files
# (these are scaled so that they are not lost in low resolution storage)
DIFF_FILE_SCALE=0.000001


from threading import Lock
class MyCache:
    """A threadsafe cache 
       with timelimit set via lazy evaluation on fetch
    """

    def __init__(self):
        self.lock = Lock()
        self.data = OrderedDict()
        self.MAXCACHE = 10
    
    def set(self, key, obj, duration=300):
        die=int(time.time())+int(duration)
        self.lock.acquire()

        self.data[key] = {"obj": obj, "die": die }
        while len(self.data.keys()) > self.MAXCACHE:
            self.data.popitem()
        self.lock.release()

    def get(self, key):
        item = self.data.get(key, None)

        if item != None:
            
            if item["die"] > int(time.time()):
                return item['obj']

            # otherwise remove it
            self.lock.acquire()
            self.data.pop(key)
            self.lock.release()

        return None


    def clear(self):
        self.lock.acquire()
        self.data = OrderedDict()
        self.lock.release()

test_cache = MyCache()

def set_cache(key, obj):
    test_cache.set(key, obj)

def get_cache(key):
    return test_cache.get(key)

def compare_strings(one, two):
    # case insensitive comparison
    return str(one).upper() == str(two).upper()


def page_dict(request):
    # extracts page dict from the request object
    return json.loads(request.POST.get('page_data', "{}"))


def run_command(cmd, use_ksh=False):
    # runs a shell command and returns output
    
    if use_ksh:
        # enforce ksh
        cmd = "ksh -c \"%s\"" % cmd

    proc = os.popen(cmd)
    data = proc.read()  
    proc.close()
    return data.strip()

def parse_node(node):
    
    # recursively parses an xml node and returns a dict
    rtn = {}

    if node.hasAttributes():
        for i in range(node.attributes.length):
            attr = node.attributes.item(i)
            rtn[attr.name] = attr.value

    if node.hasChildNodes():
        for child in node.childNodes:

        
            if child.nodeType == xml.dom.minidom.Node.TEXT_NODE:
                rtn.setdefault("Value", "")
                rtn["Value"] += child.nodeValue           # necessary as
                                                          # sometimes the chidl node text value is split as a vector 
            if child.nodeType == xml.dom.minidom.Node.ELEMENT_NODE:
                child_dict = parse_node(child)

                if child.nodeName in rtn.keys():
                    if type(rtn[child.nodeName]) != ListType:
                        # amend it so that we support more than one
                        rtn[child.nodeName] = [rtn[child.nodeName]]
                    
                    rtn[child.nodeName].append(child_dict)
                else:               
                    rtn[child.nodeName] = child_dict

    return rtn


def parse_test_file(filename, session=None):

    # parses xml files used for test storage.
    

    tests = get_cache(filename)

    # get the session key if we can

    if session:
        session["cached"] = tests!=None
        session["loading"]=session["cached"]

        session.save()

    if tests:
        print "USING CACHED DATA"

    if not tests:

        print "LOADING DATA"
        tests = []

        if session:
            session["loading"]=True
            session.save()

        print "parse_test_file loading filename %s" % filename
        fh = open(filename)
        #doc = xml.dom.pulldom.parse(filename)
        #if session:
            #    session["xml_file_handle"]=fh
            #session.save()

        #doc = xml.dom.pulldom.parse(fh)
        fsize=0
        fsize=os.path.getsize(filename)
        doc = xml.dom.pulldom.parse(fh)
        #for test in doc.getElementsByTagName("Test"):
        for event, node in doc:
            if event == xml.dom.pulldom.START_ELEMENT:

                
                if node.tagName == "Test":
        
                    doc.expandNode(node)
                    test_dict = parse_node(node)
                    # insist on several necessary keys, even if filling
                    # with blank values
                    test_dict.setdefault("Hardware", {"Machine": "Unknown",
                                                      "Platform": "Unknown"} )
                    for grid in ["SourceGrid", "TargetGrid"]:
                        test_dict.setdefault(grid, {"ShortName": "Unknown",
                                                        "Value": "Unknown"} )
                    tests.append(test_dict)
                
                # update progress
                if session:
                    loc=fh.tell()
                    pc = (100 * loc) / fsize 
                    session["progress"] = pc
                    session.save()

        set_cache(filename, tests)


    if session:
        del session["loading"]
        session.save()

    return tests
        
def get_selected_tests(request, page_data, session=None):

    # retrieve tests the user wants, either according to the local file they have
    # selected, or from the date range
    all_tests = []

    if "local_file" in page_data.keys():
        all_tests = parse_test_file(page_data["local_file"])
    else:
        start_str = page_data["start_date"]
        end_str = page_data["end_date"]

        start_date = datetime.datetime.strptime(start_str, "%Y/%m/%d")
        end_date = datetime.datetime.strptime(end_str, "%Y/%m/%d")

        test_dates = test_dates_in_range(request, start_date,  end_date)

        # now open the relevant files and read the xml
        # and form a dict of entries
        
        grid_dict = {}

        for d in test_dates:
            folder = test_folder(request, d)

            tests = parse_test_file("%s/index.xml" % folder, session)
            for t in tests:
                all_tests.append(t)


    return all_tests

def cookiestr(request):
    # returns a string of cookies from django request object suitable
    # for use in call to curl
    return ";".join(["%s=%s" % (k, request.COOKIES[k]) for k in request.COOKIES.keys()])

def http_get(url, request):
    # allows http mechanism to be changed easily
    # might try to get urllib2 or httplib2 working one day

    cmd='curl -k -s --cookie \'%s\' %s' % (cookiestr(request), url)
    data=run_command(cmd)
    try:
        return json.loads(data)
    except:
        return {}

def extract_body_from_curl_output(data):
    # removes header information from the output of curl
    # leaving only the body

    output=""
    store=False
    for count, line in enumerate(data.splitlines(True)):
        if line in ['\n', '\r\n']:
            store=True
        else:
            if store==True:
                output+=line
    return output
    
def http_post(url, data_dict, request):

    # POSTs data to url
    datastr = json.dumps(data_dict) 
    cmd='curl -k -s -i -X POST -H Accept:application/json -H Content-Type:application/json --cookie \'%s\' -d \'%s\' %s' % (cookiestr(request), 
                                                                                                                         datastr, url)
    data=run_command(cmd)
    
    try:
        return json.loads(extract_body_from_curl_output(data))
    except ValueError as e:
        print "Parsing error: %s" % e
        return {}

def new_jira_issue(request):
    
    if request.is_ajax():
        
    
        referer=request.META.get("HTTP_REFERER", "")
        

        summary = request.POST.get("summary", "No summary")
        test_uuid = request.POST.get("test_uuid", None);
        description=request.POST.get("description", "No description" );

        description = "%s\r\n\r\nTest Suite ID: %s" % (description,
                                                               test_uuid)
        
        if len(referer) > 0:
            #domain = urlparse(referer).netloc
            domain = "software.ecmwf.int/interpolation-testing"
            description = "%s\r\nLink: http://%s/search?id=%s" % (description,
                                                               domain,
                                                               test_uuid)
        
        
        #username = extract_username(request)
        # username should not be None, but in case:
            #if username == None:
                #username = "Unknown"

        
        new_dict = {
            "fields": {
                "project": {
                    "key": "REGRID"
                },
                "summary": summary,
                "issuetype": {
                  "id": 3      # "Task"
                },
                #  "reporter": {
                    #  "name": username
                #},
                "priority": {
                    "name": "Major"
                },
                "labels": [
                ],
                "environment": "Interpolation test suite",
                "description": description

                }
            }

        

        data = http_post("%s/issue" % jira_api, new_dict, request)

        if 'errors' in data:
            errors = "\r\n".join([data['errors'][k] for k in data['errors'].keys() ])
            rtn = "Jira Error: '%s'\r\n\r\nPlease report this error." % errors
        elif len(data.keys()) == 0:
            rtn = "Jira Error: Configuration problem.\r\n\r\nPlease report this error"
        else:
            rtn = ""

        return HttpResponse(rtn)


    return HttpResponse("no")

def serve_jira_issue(request):

    # returns view of jira issue

    page_data = page_dict(request)
    selected_issue_id = page_data.get("selected_issue_id", None)

    data=http_get("%s/issue/%s" % (jira_api, selected_issue_id), request)
    
    issue_title="This issue could not be found"
    issue_status="Unknown"
    
    issue_comments=[]

    try:
        issue_title=data['fields']['summary']
        issue_description=data['fields']['description']
        issue_created=data['fields']['created']
        issue_updated=data['fields']['updated']
        issue_url="%s/%s" % (jira_stem, selected_issue_id)
        issue_id="%s / %s" % (data['fields']['project']['name'], selected_issue_id)

        issue_reporter = { 
                             "icon": data['fields']['reporter']['avatarUrls']['16x16'],
                             "text": data['fields']['reporter']['displayName']
                         } 

        issue_assignee = { 
                             "icon": data['fields']['assignee']['avatarUrls']['16x16'],
                             "text": data['fields']['assignee']['displayName']
                         } 
        
        issue_priority ={   
                            "icon": data['fields']['priority']['iconUrl'],
                            "text": data['fields']['priority']['name']
                        }

        
        issue_comments=data['fields']['comment']['comments']

    except:
        pass
   

    return render_to_response("viewer/jira_issue.html",
                              locals(),
                              context_instance=RequestContext(request))

def add_jira_comment(request):
   
    # POSTs comment to jira

    if request.is_ajax():
        comment=request.POST.get("comment", "No Comment");
        selected_issue_id = request.POST.get("selected_issue_id", None)
        
        comment_object = {
                "body": comment
            ,
                            "visibility": {
                "type": "role",
                                #"value": "Administrators"
                                "value": "Developers"
                }
        }
        url="%s/issue/%s/comment" % (jira_api, selected_issue_id)
        data=http_post(url, comment_object, request)
        if 'errors' in data:
            rtn = "\r\n".join([data['errors'][k] for k in data['errors'].keys() ])
        else:
            rtn = ""

        return HttpResponse(rtn)

    return HttpResponse("no")


def serve_jira_info(request):

    if request.is_ajax():
        
        page_data = page_dict(request)

        logged_in = extract_username(request) != None
        back_page=request.META.get("HTTP_REFERER", "")
        
        test_uuid = page_data.get("test_uuid", None)

        search={
                "jql": "project = REGRID and description ~ \"%s\"" % test_uuid ,
                "startAt": 0,
                "maxResults": 999,
                "fields": [
                           "summary",
                            "status",
                            "assignee",
                            "comment"
                        ]
                }

        data=http_post("%s/search" % jira_api, search, request)
        issues=data.get('issues',[])
        if len(issues) == 1:
            message="A Jira issue exists for this test"
        else:
            message="There are %d Jira issues for this test" % len(issues)

        keys = [i['key'] for i in issues]
        keys.sort()

        titles = [i['fields']['summary'] for i in issues]
        statuses=[i['fields']['status'] for i in issues]

        selected_issue_id="None"
        if len(keys) > 0:
            selected_issue_id=keys[-1]

        can_submit_issue = test_uuid != None

        return render_to_response("viewer/jira_summary.html",
                                  locals(),
                                  context_instance=RequestContext(request))

    return HttpResponse("no")


def extract_username(request):
    
    if "WebAuth" in request.COOKIES.keys():
        # get the user id   
        webauth=request.COOKIES["WebAuth"]
        s = webauth.find("uid") 
        e = webauth.find("%20", s)
        extract=webauth[s:e]
        pair=extract.split("%3A")
        if len(pair) == 2:
            # we got the user id okay
            return pair[1]

    return None

def show_home(init_dict, request):

    init_dict = init_dict
    project_name="Interpolation"
    project_tooltip="ECMWF's new package for spectral transformation and regridding"


    from subprocess import call
    have_metview = call(["which", "metview4"]) == 0
    init_dict["have_metview"] = have_metview

    login_status="You are not logged in"
    username=extract_username(request)
    if username != None:
        login_status="You are logged in as %s" % username

    # some default image contour options
    init_dict["contour_labels"] = "on"
    init_dict["contour"] = "on"
    init_dict["shading"] = "on"

    # set a flag that states whether we're looking at "official" results
    debug_mode = "debug" in init_dict.keys()
    show_jira = settings.SHOW_JIRA_SECTION
    return render_to_response("viewer/home.html",
                              locals(),
                              context_instance=RequestContext(request))


def home(request):

    return show_home({}, request)

def uuid_to_datetime(uuid):

    try:

        epoch = (uuid.time - 0x01b21dd213814000) / 10000000
        return datetime.datetime.fromtimestamp(epoch)

    except:
        return None


def search(request):
    
    idstr = request.GET.get("id", "")
    filestr = request.GET.get("file", "")


    # clear the cache for sanity's sake
    test_cache.clear()

    if len(filestr) > 0:
        tests = parse_test_file(filestr)
        if len(tests) > 0:

            t = tests[0]
            idstr=t['Uuid']
            my_uuid = uuid.UUID("{%s}" % idstr)

            date = uuid_to_datetime(my_uuid)
            datestr=date.strftime("%Y/%m/%d")
            print "datestre from uuid is %s" % datestr

            try:
                default_image_layout = image_layout_options[0]
            except:
                default_image_layout = None

            init_dict = {
                'selected_date': datestr,
                'end_date': datestr,
                'start_date': datestr,
                'test_uuid': idstr,
                'selected_image_layout': default_image_layout,
                'debug': True,
                'local_file': filestr     # this is new and allows specific
                                          # file to be opened

            }

            mach = t['Hardware']['Machine']
            srcg = t['SourceGrid']['ShortName']
            targ = t['TargetGrid']['ShortName']
            init_dict['hardware'] = mach
            init_dict['type'] = 'machine'
            init_dict['source_grid'] = srcg
            init_dict['target_grid'] = targ

            return show_home(init_dict, request)


    
    # extract the date from this TYPE 1 uuid
    try:
        my_uuid = uuid.UUID("{%s}" % idstr)
    except:
        return HttpResponse("Could not convert supplied parameters into correct test ID");

    if my_uuid.version != 1:
        return HttpResponse("Supplied Test ID is not in the correct format. Type 1 UUID expected");

    date = uuid_to_datetime(my_uuid)

    folder = test_folder(request, date)

    # need to get a list of folders as the test might have been generated in
    # the days after the date of the folder. Never before though.
    # today's date...

    # assume tests run over a period less than a month TODO perhaps change
    # this
    search_start_date = date - datetime.timedelta(30)


    print "search start_date is %s" % search_start_date

    print "start search %s [%s] end search %s [%s]" % (search_start_date,
                                                       type(search_start_date),
                                                       date, 
                                                       type(date))
    search_dates = test_dates_in_range(request, search_start_date, date)
    print "search_dates are %s" % search_dates

    search_dates.reverse()


    found = False

    # get dates on disk between these ranges
    for d in search_dates:
        if found:
            break
        
        folder = test_folder(request, d)
        if contains_tests(folder):

            # now open the relevant files and read the xml
            # and form a dict of entries

            datestr=date.strftime("%Y/%m/%d")
        
            init_dict = {
                'selected_date': datestr,
                'end_date': datestr,
                'start_date': datestr,
                'test_uuid': idstr    
            }
        
            
            #tests = parse_test_file("%s/index.xml" % folder)
            filename = ""
            for prefix in ["", "warn_", "test_", "pass_"]:
                testname = "%s/%stest_%s.xml" % (folder, prefix, idstr)
                if os.path.isfile(testname):
                    filename = testname
                    break

            #filename="%s/test_%s.xml" % (folder, idstr)
            tests=parse_test_file(filename)
            for t in tests:
                if t['Uuid'] == idstr:
                    found = True
                    mach = t['Hardware']['Machine']
                    srcg = t['SourceGrid']['ShortName']
                    targ = t['TargetGrid']['ShortName']
                    init_dict['hardware'] = mach
                    init_dict['type'] = 'machine'
                    init_dict['source_grid'] = srcg
                    init_dict['target_grid'] = targ
                    init_dict['local_file'] = filename
                    break


    if not found:
        return HttpResponse("Could not find UUD %s in test results for dates after %s" % (idstr, date))

    return show_home(init_dict, request)


def get_namespace_data(filename, namespace):

    # get statistics here
    f = open(filename)
    count = grib_count_in_file(f)
    
    # get access gids
    gids = [grib_new_from_file(f) for i in range(count)]
    
    message_dicts = []

    for i in range(count):
        
        key_names = []

        message_dicts.append({})

        gid = gids[i]

        if gid != None:
            
            # get the stat names if we need them

            #if 0 == i:
            iterid = grib_keys_iterator_new(gid, str(namespace))

            while grib_keys_iterator_next(iterid):
                key_names.append( grib_keys_iterator_get_name(iterid) )
                
            grib_keys_iterator_delete(iterid)

            
            # now iterate over records and extract values
            for k in key_names:
                message_dicts[i][k] = grib_get(gid, k)


        grib_release(gid)

    f.close()

    return message_dicts


def get_grib_key(filename, one_indexed_message_index, keyname):

    f = open(filename)
    mcount = grib_count_in_file(f)
    if one_indexed_message_index > mcount:
        return 0

    if one_indexed_message_index <= 0:
        return 0

    gid_list = [grib_new_from_file(f) for i in range(mcount)]

    our_gid = gid_list[one_indexed_message_index-1]
    key_value = grib_get(our_gid, keyname)

    for gid in gid_list:
        grib_release(gid)

    f.close()

    return key_value

def get_grib_length(filename, one_indexed_message_index):

    f = open(filename)
    mcount = grib_count_in_file(f)
    if one_indexed_message_index > mcount:
        return 0

    if one_indexed_message_index <= 0:
        return 0

    gid_list = [grib_new_from_file(f) for i in range(mcount)]

    our_gid = gid_list[one_indexed_message_index-1]
    number_of_values = len(grib_get_values(our_gid))

    for gid in gid_list:
        grib_release(gid)

    f.close()

    return number_of_values


def grib_count_differences(file1, file2, one_indexed_message_index, atol):

    files = []
    gids = []
    
    start = time.time()
    mindiff=None
    maxdiff=None

    for inp in (file1, file2):
        f = open(inp)
        mcount = grib_count_in_file(f)
        if one_indexed_message_index > mcount:            
            raise
        if one_indexed_message_index <= 0:
            raise
        
        # skip to correct index
        index = 1
        while index < one_indexed_message_index:
            grib_new_from_file(f)
            index += 1

        g = grib_new_from_file(f)

        files.append(f)
        gids.append(g)

    count = 0

    if len(gids) == 2:

        vals0 = grib_get_values(gids[0])
        vals1 = grib_get_values(gids[1])
        

        arr0 = numpy.array(vals0)
        arr1 = numpy.array(vals1)
        #count = numpy.sum(vals0!=vals1)
        count = numpy.sum(abs(vals1-vals0)>=atol)
        arrd=vals1-vals0
        mindiff = numpy.min(arrd)
        maxdiff = numpy.max(arrd)

    for g in gids:
        grib_release(g)
    for f in files:
        f.close()

    return count, mindiff, maxdiff


def grib_get_differences(file1, file2, one_indexed_message_index, atol,                         
                         one_indexed_start_pos=0, ndiff=10000, disable_comparison=False):

    start = time.time()
    files = []
    gids = []
    
    for inp in (file1, file2):
        f = open(inp)
        mcount = grib_count_in_file(f)
        if one_indexed_message_index > mcount:
            raise
        if one_indexed_message_index <= 0:
            raise
        
        # skip to correct index
        index = 1
        while index < one_indexed_message_index:
            grib_new_from_file(f)
            index += 1

        g = grib_new_from_file(f)

        files.append(f)
        gids.append(g)

    ldata = ["  Lat\tLon\tValue"]
    rdata = ["  Lat\tLon\tValue"]

    
    if len(gids) == 2:

        iter1 = grib_iterator_new(gids[0], 0)
        iter2 = grib_iterator_new(gids[1], 0)
        
        # skip
        '''
        for s in xrange(1, one_indexed_start_pos):
            grib_iterator_next(iter1)
            grib_iterator_next(iter2)
        '''

        count=0
        diffs = 0
        while len(ldata) < ndiff:
            result1 = grib_iterator_next(iter1)
            result2 = grib_iterator_next(iter2)

            if not result1: break
            if not result2: break

            [lat1, lon1, val1] = result1
            [lat2, lon2, val2] = result2
            
            if disable_comparison or abs(val1 - val2) >= atol:
                diffs += 1
                if diffs >= one_indexed_start_pos:
                    ldata.append("  %.9f  %.9f  %s" % (lat1, lon1, val1))
                    rdata.append("  %.9f  %.9f  %s" % (lat2, lon2, val2))
            count+=1


        grib_iterator_delete(iter1)
        grib_iterator_delete(iter2)
        


    for g in gids:
        grib_release(g)
    for f in files:
        f.close()

    return (ldata, rdata)

def serve_png(request):
    
    #filename, ext = os.path.splitext(request.REQUEST.get('name', 'none'))
    filename = request.REQUEST.get("f", "Missing")

    image_data = open("%s" % filename, "rb").read()
    return HttpResponse(image_data, mimetype="image/png")


def daterange(start_date, end_date):
    for n in range(1 + (end_date - start_date).days):
        yield start_date + datetime.timedelta(n)


def set_folder_path(request):

    if "folder_path" in request.POST:
        request.session["results_folder"] = request.POST["folder_path"]

    return list_view(request)

def get_results_folder(request):

    results_folder = settings.RESULTS_FOLDER

    # has the user chosen a new one?
    
    
    if "results_folder" in request.session:
        results_folder = request.session["results_folder"]
    
    return results_folder

def test_folder(request, test_date):

    #results_folder="/scratch/ma/maf/scratchdir/ecRegrid/bigtest/output"
    #results_folder="/scratch/ma/maf/scratchdir/ecRegrid/suite/output"
    #results_folder="/tmp/maf/bigtest/output"
    
    results_folder=get_results_folder(request)

    datestr=test_date.strftime("%Y%m%d")
    test_folder="%s/%s" % (results_folder, datestr)
    return test_folder
    
##from __future__ import generators
#from lxml import etree

import glob
def list_view(request):

    results_folder=get_results_folder(request)
    # get list of dates from the folders there
    
    #if "folder" in request.GET:
    #    results_folder = request.GET["folder"]

    msg = "Choose a folder"

    try:
        folders = sorted(os.listdir(results_folder))
    except:
        pass
        msg = "That folder doesn't exist. Please try again"

    
    if request.method == "POST":

        warnings_only = request.POST.get("warnings_only", None)

        prefix = "pass_test_" # or "warn_test_"
        if warnings_only:
            prefix = "warn_test_"
            
        # they requested a particular folder to view
        folder_selected = request.POST.get("folder_selected", None)
        if folder_selected:
            # we list it
            folder_full_path = os.path.join(results_folder, folder_selected)
            #####numpy_list(folder_full_path)
            #items = os.listdir(folder_full_path)
            #msg = "folder %s has %d items" % (folder_selected, len(items))
            #item_list = dirwalk(folder_full_path)
            #prefix = "pass_test_" # or "warn_test_"
            lp = len(prefix)
            item_list = [os.path.splitext(os.path.basename(g))[0][lp:] for g in glob.glob('%s/%s*.xml' % (folder_full_path, prefix))]

            msg = "Found %d files in %s/*_test_*.xml" % (len(item_list),
                                                  folder_full_path)
        else:
            #msg = "folder selected empty"
            pass




    return render_to_response("viewer/list_view.html",
                              locals(),
                              context_instance=RequestContext(request))
    
def contains_tests(folder):

    # first, is it a folder?
    if os.path.isdir(folder):                  
        if "index.xml" in os.listdir(folder):
            return True

    return False

def test_dates_in_range(request, start_date, end_date):
    # get dates on disk between these ranges
    tests = []
    for d in daterange(start_date, end_date):
        print "test folder for date %s is %s" % (d, test_folder(request, d))
        if contains_tests(test_folder(request, d)):
            tests.append(d)

    return tests



def get_test_dates(request):

    # returns the valid test dates (for which data is available) for the month
    # and year passed. Must include the target control as part of the json
    # array that we return
    if request.is_ajax():
        
        month = request.POST.get('month')   # NB one indexed month
        year = request.POST.get('year')
        target = request.POST.get("target")
        

        date_strings = []

        start_date = datetime.datetime(int(year), int(month), 1, 0, 0, 0)
        end_date = start_date + relativedelta(months=+1)

        for d in test_dates_in_range(request, start_date, end_date):
            date_strings.append(d.day)

        response = {'target': target, 'dates': date_strings }
        return HttpResponse(json.dumps(response))

def get_load_progress(request):

    loading=request.session.get("loading", False)
    cached=request.session.get("cached", False)
    value=request.session.get("progress", 0)
    
    if loading:
        # limit the range to 80 pc
        val = 0.8 * float(value)
        value = int(0.8 * float(value))
    
    else:
        if not cached:
            # we have just loaded... compress to the final 20pc
            if value > 0:
                value = int(80.0 + float(value) * 0.2)

    return HttpResponse(value) 

def serve_hardware(request):

    # returns all info on platforms and machines available for a particular
    # test date
    

    if request.is_ajax():

        start_str = request.POST.get('start_date')
        end_str = request.POST.get('end_date')

        page_data = page_dict(request)
        platforms = []
        machines = []

        # also get information on the versions discovered in the test file
        #interpolators = {}
        interpolators = OrderedDict()

        tests = get_selected_tests(request, page_data, request.session)
        test_count = len(tests)
        print "There are %d tests in the selected date range" % test_count

        for count, t in enumerate(tests):
            #time.sleep(0.02)#for testing
            pc = (100 * count) / test_count 
            request.session["progress"] = pc
            request.session.save()

            try:
                mach = t['Hardware']['Machine']
                plat = t['Hardware']['Platform']
            except KeyError:
                mach = "Unknown"
                plat = "Unknown"

            if mach not in machines:
                machines.append(mach)
            if plat not in platforms:
                platforms.append(plat)
           
            if "Interpolation" in t:
                for interp in t['Interpolation']:
                    try:
                        name = interp['Name'].upper()
                        ver = interp['Version']
                    except:
                        print "ERROR: getting name or version of Interpolator from test"

                    interpolators.setdefault(name, [])
                    if ver not in interpolators[name]:
                        interpolators[name].append(ver)

            else:
                print "ERROR: Interpolation not found in test"
    
        platforms.sort()

        print "interpolators has keys %s" % interpolators.keys()

        # remove the progress key
        del request.session["progress"]
        request.session.save()

        selection = page_data.get("hardware", "")
        
        if len(platforms) > 0 and (len(selection) == 0 or selection not in platforms):
            selection = platforms[0]

        all_="All"
        selected_version=page_data.get("selected_version", all_)
        selected_interpolator=page_data.get("selected_interpolator", all_)
        interpolators[all_]=[all_]
        


        # return a rendered grid for loading
        # test render a page of detail
        return render_to_response("viewer/machines.html",
                                    locals(),
                                  context_instance=RequestContext(request))


def serve_source_grids(request):

    if request.is_ajax():
        
        page_data = page_dict(request)
        
        #hardware = request.POST.get("hardware")
        #hardware_type = request.POST.get("type")
        hardware = page_data.get("hardware")
        hardware_type = page_data.get("type")

        selected_interpolator = page_data.get("selected_interpolator", "")
        selected_version = page_data.get("selected_version", "All")

        # NB might need a local dict to do this from a form in the page for
        # allowing same user to have different views.
        #request.session["selection"] = hardware

        # get the source grids for this particular hardware
       
        grid_dict = {}
        tests = get_selected_tests(request, page_data)

        for t in tests:
            keep = False
            if compare_strings(selected_interpolator, "All"):
                keep = True
            else:
                for interp in t['Interpolation']:
                    if compare_strings(interp['Name'], selected_interpolator):
                        if compare_strings(interp['Version'], selected_version):
                            keep = True
                        
            if not keep:
                continue

            if "machine" == hardware_type:
                if t['Hardware']['Machine'] != hardware:
                    continue
            if "platform" == hardware_type:
                if t['Hardware']['Platform'] != hardware:
                    continue

            status = t['Status']
            # statuses are bit packed. So 1<<PASS === 1<<0 == 1

            src_name = t['SourceGrid']['Value']
            src_key = t['SourceGrid']['ShortName']
            grid_dict.setdefault(src_key, { "name": src_name, "fails": 0, "targets": {} })
            
            tar_name = t['TargetGrid']['Value']
            grid_dict[src_key]["targets"].setdefault(tar_name, {"tests": 0, "passes": 0, "fails": 0})
            grid_dict[src_key]["targets"][tar_name]["tests"] += 1       
            
            # status is a status code in the UNIX style with 0 a pass
            # but the values are bit-packed into an int

            if int(status) == 1<<0:
                grid_dict[src_key]["targets"][tar_name]["passes"] += 1
            else:
                grid_dict[src_key]["targets"][tar_name]["fails"] += 1
                grid_dict[src_key]["fails"] += 1
        
        selection = page_data.get("source_grid", "")
        if len(grid_dict.keys()) > 0 and (len(selection) == 0 or selection not in grid_dict.keys()):
            selection = grid_dict.keys()[0]

        return render_to_response("viewer/source.html",
                                    locals(),
                                  context_instance=RequestContext(request))


def serve_target_grids(request):

    if request.is_ajax():

        page_data = page_dict(request)
        source_grid = page_data.get("source_grid")
        source_grid_full = page_data.get("source_grid_full")

        hardware = page_data.get("hardware")
        hardware_type = page_data.get("type")

        start_str = page_data.get("start_date", "")
        end_str = page_data.get("end_date", "")

        all_="All"
        selected_version=page_data.get("selected_version", all_)
        selected_interpolator=page_data.get("selected_interpolator", all_)

        try:
            start_date = datetime.datetime.strptime(start_str, "%Y/%m/%d")
            end_date = datetime.datetime.strptime(end_str, "%Y/%m/%d")
        except:
            start_date = datetime.datetime.now()
            end_date = start_date
        
        selection = page_data.get("target_grid", "")

        #test_dates = test_dates_in_range(request, start_date,  end_date)
        test_uuid = page_data.get("test_uuid", "None")

        target_grids = {} 
        test_headers = []
        ignore_keys = []
        

        tests = get_selected_tests(request, page_data)

        for t in tests:
            
            keep = False
            if compare_strings(selected_interpolator, "All"):
                keep = True
            else:
                for interp in t['Interpolation']:
                    if compare_strings(interp['Name'], selected_interpolator):
                        if compare_strings(interp['Version'], selected_version):
                            keep = True
            if not keep:
                continue

            if "machine" == hardware_type:
                if t['Hardware']['Machine'] != hardware:
                    continue
            if "platform" == hardware_type:
                if t['Hardware']['Platform'] != hardware:
                    continue
            
            
            status = t['Status']
            src_name = t['SourceGrid']['Value']
            src_key = t['SourceGrid']['ShortName']
            
            if src_key == source_grid:
                tar_key = t['TargetGrid']['ShortName']
                
                tar_name = t['TargetGrid']['Value']
                target_grids.setdefault(tar_key, {})
                target_grids[tar_key]["name"] = tar_name
                
                target_grids[tar_key].setdefault("tests", [])
                
                
                test_dict = OrderedDict()
                 
                test_dict["status bits"] = t.get("Status", "?")
                test_dict["reason"] = t.get("StatusLine", "Unknown")
                test_dict["date"] = t["Date"]
                test_dict["time"] = t["Time"]
                test_dict["platform"] = t["Hardware"]["Platform"]
                test_dict["machine"] = t["Hardware"]["Machine"]
                params = [ t['Parameter']['Value'] ]
                test_dict["parameters"] = ', '.join(params)
                test_dict["name"] = t["Name"]
                test_dict["uuid"] = t["Uuid"]
               
                 
                target_grids[tar_key]["tests"].append(test_dict)
                for k in test_dict.keys():
                    if k not in test_headers and k not in ignore_keys:
                        test_headers.append(k)

                if selection == "":
                    selection == tar_key

                if selection == tar_key:
                    if test_uuid == "None":
                        test_uuid = t['Uuid']
                
               


        if len(target_grids.keys()) > 0 and (len(selection) == 0 or selection not in target_grids.keys()):
            selection = target_grids.keys()[0]


        # now decide on uuid selection where none exists
        #if test_uuid == "None" and selection != "":
            #    if len(target_grids[selection]["tests"]) > 0:
                #    test_uuid=target_grids[selection]["tests"][0]["uuid"]

        test_header_titles = [t.title() for t in test_headers]
        hidden_uuid_column = test_header_titles.index("Uuid")

        return render_to_response("viewer/target.html",
                                  locals(),
                                  context_instance=RequestContext(request))

def is_wind(test_list):
    # checks whether combination of parameters means we render as wind
    if "u" in test_list and "v" in test_list:
        return True
    if "10u" in test_list and "10v" in test_list:
        return True

def serve_detail(request):

    namespaces = ["statistics", "ls", "mars", "parameter", "time", "vertical" ]
    dump_styles = ["standard", "octet", "debug"]
    compare_styles = ["standard", "verbose", "values"]

    resolutions = ["low", "high"]
    
    
    projections=[os.path.splitext(fi)[0] for fi in os.listdir("%s/magjson/projection" % settings.STATIC_ROOT)]

    script_types = []
    raw_input_types = []

    log_types = []
    data_types = ["Differences", "All"]
    image_options = ["shading", "contour", "legend", "contour_labels", "values"]
    selected_image_options = []
    #diff_types = ["data"]
    #data_diff_type = "data"
    diff_types = []
    data_diff_type = None

    test_uuid = "None"
    selected_date = "None"

    if request.is_ajax():
        
        page_data = page_dict(request)
    
        have_metview  = page_data.get("have_metview", False)
        
        tolerance = page_data.get("tolerance", 0.0)

        for opt in image_options:
            if page_data.get(opt, "off") == "on":
                selected_image_options.append(opt)

        data_diff_type = page_data.get("data_diff_type", None)

        selected_image_layout = page_data.get("selected_image_layout", None)
        if selected_image_layout == None and len(image_layout_options) > 0:
            selected_image_layout = image_layout_options[0]

        active_message=int(page_data.get("active_message", 0))
    
        grib_dump_style = page_data.get("grib_dump_style", None)
        if grib_dump_style == None and len(dump_styles) > 0:
            grib_dump_style = dump_styles[0]
            
        grib_compare_style = page_data.get("grib_compare_style", None)
        if grib_compare_style == None and len(compare_styles) > 0:
            grib_compare_style = compare_styles[0]
        

        source_grid_full = page_data.get("source_grid_full")
        target_grid = page_data.get("target_grid")
        target_grid_full = page_data.get("target_grid_full")
        selected_namespace = page_data.get("selected_namespace", "statistics")
        test_uuid = page_data.get("test_uuid", test_uuid)

        selected_date = page_data.get("selected_date", selected_date)
        detail_tab = int(page_data.get("detail_tab", 0))
        selected_script_type=page_data.get("selected_script_type", None)
        selected_log_type=page_data.get("selected_log_type", None)
        selected_data_type=page_data.get("selected_data_type", None)
        if selected_data_type == None and len(data_types) > 0:
            selected_data_type = data_types[0]

        selected_raw_input_type=page_data.get("selected_raw_input_type", None)

        resolution=page_data.get("resolution", resolutions[0] if len(resolutions) > 0 else None)
        projection=page_data.get("projection", None)

        test_date_str = "Unknown"

        folder, out_dict = get_test_data(request, page_data, "Output")
        folder, in_dict = get_test_data(request, page_data, "Input")

        grib_files = {}
        grib_labels = []


        diff_files = {}
        diff_labels = []

        for key in out_dict:
        
            output_files = out_dict[key]['data']
            # we expect only one of each type but in case there are
            # more than one we handle that (eventually)
            for filedict in output_files:
                out_class=filedict.get("Class", "Missing")
                if key.upper() == "COMPARISON":
                    
                    if compare_strings(out_class, "Data"):
                        out_type = filedict.get("Type", "")
                        if compare_strings(out_type, "Grib"):
                            diff_type = filedict.get("Label", "")
                            if len(diff_type) > 0 and diff_type not in diff_types:
                                diff_types.append(diff_type)
                                diff_files[diff_type] = filedict.get("File")
                    
                else:
                    if compare_strings(out_class, "Log"):
                        out_type = filedict.get("Type", "")
                        if len(out_type) > 0 and out_type not in log_types:
                            log_types.append(out_type)
                    elif compare_strings(out_class, "Data"):
                        out_type = filedict.get("Type", "")
                        if compare_strings(out_type, "Grib"):
                            grib_file = filedict.get("File")

                            grib_labels.append(key)
                            grib_files[key]=grib_file
                            
                        
                            try:
                                with open(grib_file) as f:
                                    grib_message_count = grib_count_in_file(f)

                            except:
                                grib_message_count = 0
                        
                            grib_messages = [i for i in range(0, grib_message_count)]
                            if active_message >= grib_message_count:
                                active_message = 0

                        
        diff_labels = diff_types

        wind=False

        if len(grib_labels) > 0:

            cmd = "grib_get -P shortName %s" % grib_files[grib_labels[0]]
            params = run_command(cmd)
            param_list = params.split("\n")

            # if params was anything at all then it now has length 1 or more    
            # remove the duplicates
            test_list = list(set(param_list))
            if len(test_list) > 1:
                wind=is_wind(test_list)
                '''
                # we have more than one parameter
                if "u" in test_list and "v" in test_list:
                    wind=True
                if "10u" in test_list and "10v" in test_list:
                    wind=True
                '''

        for key in in_dict:

            # don't want the comparison file for this view
            if key.upper() == "COMPARISON":
                continue
            input_files = in_dict[key]['data']
            for indict in input_files:  
                in_class=indict.get("Class", "Missing")
                if compare_strings(in_class, "Script"):
                    in_type = indict.get("Type", "")
                    if len(in_type) > 0 and in_type not in script_types:
                        script_types.append(in_type)            


        tests = get_selected_tests(request, page_data)
        
        for t in tests:
            if t['Uuid'] == test_uuid:
                test_time_str = t['Time']
                date_obj = datetime.datetime.strptime(t['Date'], "%Y%m%d") 
                test_date_str = date_obj.date().isoformat()
                raw_data = t.get('RawData', {})

                input_dict = raw_data.get("Input", {})
                raw_type = input_dict.get("Type", None)
                if raw_type and raw_type not in raw_input_types:
                    raw_input_types.append(raw_type)


        if selected_raw_input_type == None:
            if len(raw_input_types) > 0:
                selected_raw_input_type = raw_input_types[0]


        if selected_script_type == None:
            if len(script_types) > 0:
                selected_script_type = script_types[0]
        if selected_log_type == None:
            if len(log_types) > 0:
                selected_log_type = log_types[0]
        if data_diff_type == None:
            if len(diff_types) > 0:
                data_diff_type = diff_types[0]

        if projection == None:
            projection = "global" if "global" in projections else projections[0] if len(projections) > 0 else None


        page_params = {"image_layout_options": image_layout_options}

        # test render a page of detail

        co_scrolling = page_data.get("co_scrolling")
        
        page_params.update(locals())
        return render_to_response("viewer/detail.html",
                            page_params,
                          context_instance=RequestContext(request))


    return HttpResponse("not allowed")

def serve_raw_input(request):

    if request.is_ajax():

        page_data = page_dict(request)
        test_uuid = page_data.get("test_uuid", "None")
        tests = get_selected_tests(request, page_data)
        selected_raw_input_type = page_data.get("selected_raw_input_type", None)
        selected_date=page_data.get("selected_date", None)
        data = "No data can be found"
        if selected_date:
            date_obj = datetime.datetime.strptime(selected_date, "%Y%m%d")
            folder = test_folder(request, date_obj)

            for t in tests:
                if t['Uuid'] == test_uuid:
                    raw_data = t.get('RawData', {})
                    input_dict = raw_data.get("Input", {})
                    raw_type = input_dict.get("Type", None)
                    if raw_type and raw_type == selected_raw_input_type:
                        the_file = input_dict.get("File", None)
                        if the_file:
                            data = open("%s/%s" % (folder, the_file),"r").read()

                    break

        label = "Source request"
        return render_to_response("viewer/single.html",
                                  locals(),
                                  context_instance=RequestContext(request))

    return HttpResponse("not allowed")


def sort_into_tables(names, vals):

    # handles breaking up of tables into an array of sub-tables for simple display
    # if names are [parameter_index]
    # values are [message_index, parameter_index]
    # the tables will be returned as
    # names as [table_index, parameter_index]
    # values as [table_index, message_index, parameter_index]

    # we want a max of four columns. some algo to work out reasonable column counts to ensure that
    max_columns=4
    tables,rem=divmod(len(names), max_columns)
    if rem != 0:
        tables+=1 
    
    columns,rem=divmod(len(names), tables)
    if rem != 0:
        columns+=1

    name_tables=[]
    vals_tables=[]

    for i in range(tables):
        name_tables.append([])
        vals_tables.append([ [] for k in range(len(vals))  ])
        for j in range(columns):
            index=(i*columns)+j
            if index < len(names):
                name_tables[i].append(names[index])
                for k in range(len(vals)):
                    vals_tables[i][k].append(vals[k][index])
                    
    return (name_tables, vals_tables)


def get_test_data(request, page_data, tag_to_match="Output"):
    

    # option to get caching if we need to
    # though if it's fast enough no matter

    # from the input dict retrieve relevant files from the interpolation
    test_uuid = page_data.get("test_uuid", "None")
    selected_date = page_data.get("selected_date", "None")
    folder = ""
    rtn_dict = {}

    tests = []



    # if local file selected, read from that rather than from the date range
    if "local_file" in page_data.keys():
        filename = page_data["local_file"]
        folder = os.path.dirname(filename)

        tests = parse_test_file(filename)
    else:

        if selected_date != "None":

            date_obj = datetime.datetime.strptime(selected_date, "%Y%m%d")
            folder = test_folder(request, date_obj)
            if contains_tests(folder):
                # now open the relevant files and read the xml
                # and form a dict of entries
                tests = parse_test_file("%s/index.xml" % folder)

    # now traverse the tests...

    for t in tests:

        if t['Uuid'] == test_uuid:
            test_time_str = t['Time']
            interp = t['Interpolation'] 
             
            for i in interp:
                name = i['Name']
                version = i["Version"]
                key = "%s %s" % (name, version)
                rtn_dict.setdefault(key, { "data": []})
                rtn_dict[key]["version"]=version
                for k in i.keys():
                    if k not in ["Version", "Name"]:
                        if type(i[k]) not in [ListType, DictType]:
                            rtn_dict[key][k]=i[k]
                
                if tag_to_match in i.keys():
                    obj = i[tag_to_match] if type(i[tag_to_match]) == ListType else [ i[tag_to_match] ]
                else:
                    obj = {}
                
                for o in obj:
                    file_dict = {}  
                    for k in o.keys():
                        file_dict[k] = o[k]
                        if compare_strings(k, "FILE"):
                            file_dict[k] = "%s/%s" % (folder, o[k])
                    rtn_dict[key]['data'].append(file_dict)     
                
            if "Comparison" in t.keys():    
                compar = t['Comparison']
                
                rtn_dict.setdefault("Comparison", {"data": []})
                if tag_to_match in compar.keys():
                    obj = compar[tag_to_match] if type(compar[tag_to_match]) == ListType else [ compar[tag_to_match] ]
                    for o in obj:
                        file_dict = {}  
                    
                        for k in o.keys():
                            file_dict[k] = o[k]                        
                            if compare_strings(k, "FILE"):
                                file_dict[k] = "%s/%s" % (folder, o[k])
                        rtn_dict["Comparison"]["data"].append(file_dict)     
                        
                         
                        
    return (folder, rtn_dict)

def serve_summary(request):

    if request.is_ajax():

        page_data = page_dict(request)
        
        folder, out_dict = get_test_data(request, page_data, "Output")

        labels = []
        files = []        

        data = []
        
        for key in out_dict:

            if key.upper() == "COMPARISON":
                continue

            output_files = out_dict[key]['data']
            
            # we expect only one of each type but in case there are
            # more than one we handle that (eventually)
            d = OrderedDict()

            d['Peak Memory (B)']=out_dict[key].get("MemoryBytes", "Unknown")
            d['Timer (ms)']=out_dict[key].get("TimerMs", "Unknown")

            for filedict in output_files:
                file_type=filedict.get("Type", "Missing")

                if file_type.upper() == "GRIB":
                    labels.append(key)
                    if "File" in filedict.keys():
                        this_grib=filedict.get("File")
                        files.append(filedict.get("File"))
                        d['GRIB Size (B)']=int(run_command("wc -c < %s" % this_grib))

            data.append(d)
            

        if len(labels) == 2 and len(files) == 2:
            

            left_label=labels[0]
            right_label=labels[1]
            left_file = files[0]
            right_file = files[1]
            
            
            left_dict = data[0] 
            right_dict = data[1]

            
            equal_string = "Identical"

            centre_dict = OrderedDict()
            for k in left_dict.keys():            
                if k in right_dict.keys():
                    try:
                        if type(right_dict[k]) in (FloatType, IntType):
                            centre_dict[k] = right_dict[k]-left_dict[k]
                        else:
                            if right_dict[k] != left_dict[k]:
                                centre_dict[k] = "Different"
                            else:
                                centre_dict[k] = equal_string
                    except:
                        centre_dict[k] = "N/A"


            headers = ["key", "value"]
            diff_label = "%s - %s" % (right_label.title(), left_label.title())

            # difference values of the following values will be un-highlighted
            equalvals = [0, equal_string]


            # list of values that we equate as "Equal"
            return render_to_response("viewer/compare_tables.html",
                                        locals(),
                                      context_instance=RequestContext(request))

        return HttpResponse("An error occurred while accessing data")



def serve_log(request):

    if request.is_ajax():

        page_data = page_dict(request)
        selected_log_type = page_data.get("selected_log_type", "MARS")

        folder, out_dict = get_test_data(request, page_data, "Output")

        labels = []
        files = []        

        for key in out_dict:
        
            # don't want the comparison file for this view
            if key.upper() == "COMPARISON":
                continue

            output_files = out_dict[key]['data']
            # we expect only one of each type but in case there are
            # more than one we handle that (eventually)
            for filedict in output_files:
                file_type=filedict.get("Type", "Missing")
                if file_type.upper() == selected_log_type.upper():
                    labels.append(key)
                    if "File" in filedict.keys():
                        files.append(filedict.get("File"))
                    
        if len(labels) == 2 and len(files) == 2:

            left_label=labels[0]
            right_label=labels[1]
            left_file = files[0]
            right_file = files[1]
            
            try:
                left_data = open(left_file,"r").read()
            except IOError as e:
                left_data = "Could not open %s" % left_file 

            try:
                right_data = open(right_file,"r").read()
            except IOError as e:
                right_data = "Could not open %s" % right_file 

            if compare_strings(selected_log_type, "environment"):

                # markup the environment variables

                keywords = ["emos", "scin", "ecregrid"]

                left_out_data = left_data.split("\n")
                right_out_data = right_data.split("\n")

                for i in xrange(0, len(left_out_data)):
                    left_out_data[i] = markup_match(left_out_data[i], keywords)
                for i in xrange(0, len(right_out_data)):
                    right_out_data[i] = markup_match(right_out_data[i], keywords)
                    
                left_data = "\n".join(left_out_data)
                right_data = "\n".join(right_out_data)
 

            return render_to_response("viewer/side_by_side.html",
                                        locals(),
                                      context_instance=RequestContext(request))

        return HttpResponse("%s log data could not be found for this test" % selected_log_type)
        

def serve_script(request):

    if request.is_ajax():
        page_data = page_dict(request)
        selected_script_type = page_data.get("selected_script_type", "MARS")
        

        folder, in_dict = get_test_data(request, page_data, "Input")
        
        labels = []
        scripts = []
        for key in in_dict:
            if key.upper() == "COMPARISON":
                continue

            input_files = in_dict[key]['data']
            for filedict in input_files:
                file_type = filedict.get("Type", "Missing")
                if file_type.upper() == selected_script_type:
                    labels.append(key)
                    if "File" in filedict.keys():   
                        scripts.append("%s" % filedict.get("File"))

        
        if len(labels) == 2 and len(scripts) == 2:

            left_label=labels[0]
            right_label=labels[1]
            left_file = scripts[0]
            right_file = scripts[1]
            
            try:
                left_data = open(left_file,"r").read()
            except IOError as e:
                left_data = "Could not open %s" % left_file 

            try:
                right_data = open(right_file,"r").read()
            except IOError as e:
                right_data = "Could not open %s" % right_file 


            return render_to_response("viewer/side_by_side.html",
                                        locals(),
                                      context_instance=RequestContext(request))

        return HttpResponse("An error occurred while accessing data")


def markup_match(s, keywords):
    # marks up a line of a log output if a match to a keyword occurs
    for k in keywords:
        if s.upper().find(k.upper()) >= 0:
            return "<span class='hilite'>%s</span>" % s

    return s

def markup_diffs(s1, s2):

    # takes two equal length strings and marks up the difference in html style
    if len(s1) != len(s2):
        pass    
        #return

    length = min(len(s1), len(s2))

    diffs = []
    for i in range(0, length):
        if s1[i] != s2[i]:
            diffs.append(int(i))
    try:
        lo = min(diffs)
        hi = max(diffs)+1
    except:
        lo = 0
        hi = 0
    
    s1 = s1.replace(s1[lo:hi], "<span class='hilite'>%s</span>" % s1[lo:hi])
    s2 = s2.replace(s2[lo:hi], "<span class='hilite'>%s</span>" % s2[lo:hi])
    
    return s1, s2

def serve_data_frame(request):

    if request.is_ajax():

        paginated = True
        current_page = 1
        npages = 100

        page_data = page_dict(request)
        selected_data_type = page_data.get("selected_data_type", "All")
        tolerance = page_data.get("tolerance", "1.0")
        active_message=int(page_data.get("active_message", 0))

        try:
            diff_tolerance = float(tolerance)
        except:
            diff_tolerance = 0.0

        

        folder, out_dict = get_test_data(request, page_data, "Output")

        grib_files = []
        for key in out_dict:
        
            # don't want the comparison file for this view
            if key.upper() == "COMPARISON":
                continue
        
            
            output_files = out_dict[key]['data']
            # we expect only one of each type but in case there are
            # more than one we handle that (eventually)
            for filedict in output_files:
                out_class=filedict.get("Class", "Missing")
                if compare_strings(out_class, "Data"):
                    
                    
                    # look at grib file and get the length
                    the_file = filedict.get("File", "None")
                    grib_files.append(the_file)
    

        # here we have the grib files
        if len(grib_files) == 2:
                    
            if selected_data_type != "Differences":
                diff_tolerance = 0.0

            
            #numberOfPoints, mindiff, maxdiff = (0,0,0) # grib_count_differences(grib_files[0],
            try:
                numberOfPoints, mindiff, maxdiff = grib_count_differences(grib_files[0],
                                                        grib_files[1],
                                                        active_message+1,
                                                        diff_tolerance)
            except:
                # return the length.... it will be because the files are
                # different lengths and so all values will be different anyway
                numberOfPoints, mindiff, maxdiff = (get_grib_length(grib_files[0],1),-1,-1) # grib_count_differences(grib_files[0],



            
      
            # we want only N per page 
            chunk = 500
            npages = 1 + int(numberOfPoints) / chunk

            #if npages == 0:
                #    npages = 1
                        
            #chunk = int(numberOfPoints) / npages
            #chunk = 10 * int(chunk / 10)

            current_page = 1
            start_val = 1 + (current_page * chunk)
            paginated = True

        return render_to_response("viewer/data_view.html",
                                      locals(),
                                      context_instance=RequestContext(request))


def serve_data(request):

    if request.is_ajax():

        page_data = page_dict(request)
            
        folder, out_dict = get_test_data(request, page_data, "Output")
        
        selected_data_type = page_data.get("selected_data_type", "All")
        active_message=int(page_data.get("active_message", 0))
        tolerance = page_data.get("tolerance", 0.0)
        try:
            tolerance = float(tolerance)
        except:
            tolerance = 0.0
        

        labels = []
        files = []

        for key in out_dict:

            # don't want the comparison file for this view
            if key.upper() == "COMPARISON":
                continue

            output_files = out_dict[key]['data']
            # we expect only one of each type but in case there are
            # more than one we handle that (eventually)
            for filedict in output_files:
                file_type=filedict.get("Type", "Missing")
                if file_type.upper() == "GRIB":
                    labels.append(key)
                    if "File" in filedict.keys():
                        files.append(filedict.get("File"))

        differences_only = selected_data_type == "Differences"
        
        if len(labels) == 2 and len(files) == 2:

            left_label=labels[0]
            right_label=labels[1]
            left_file = files[0]
            right_file = files[1]

            show_page = int(request.POST.get("show_page", "1"))
            show_lines = int(request.POST.get("show_lines", "500"))
            
            start_val = 1 + (show_page - 1) * show_lines

            msg_1_indexed = active_message + 1

            disable_comparison = not differences_only

            left_out_data, right_out_data = grib_get_differences(left_file,
                                                                 right_file,
                                                                 msg_1_indexed,
                                                                 tolerance,
                                                                 start_val,
                                                                 show_lines,
                                                                 disable_comparison)

            for i in xrange(0, len(left_out_data)):
                left_out_data[i], right_out_data[i] = markup_diffs(left_out_data[i], right_out_data[i])

            left_data = "\n".join(left_out_data)
            right_data = "\n".join(right_out_data)

            return render_to_response("viewer/side_by_side.html",
                                      locals(),
                                      context_instance=RequestContext(request))

        return HttpResponse("An error occurred while accessing data")

        
def serve_grib_ls(request):

    if request.is_ajax():

        page_data = page_dict(request)
        namespace = page_data.get("selected_namespace", "statistics")
        
        active_message = int(page_data.get("active_message", 0))
        
        folder, out_dict = get_test_data(request, page_data, "Output")

        labels = []
        files = []        
        

        for key in out_dict:
        
            # don't want the comparison file for this view
            if key.upper() == "COMPARISON":
                continue

            output_files = out_dict[key]['data']
            # we expect only one of each type but in case there are
            # more than one we handle that (eventually)
            for filedict in output_files:
                file_type=filedict.get("Type", "Missing")
                if file_type.upper() == "GRIB":
                    labels.append(key)
                    if "File" in filedict.keys():
                        files.append(filedict.get("File"))
                    
        if len(labels) == 2 and len(files) == 2:
            
            left_label=labels[0]
            right_label=labels[1]
            left_file = files[0]
            right_file = files[1]
            
            left_dicts = get_namespace_data(left_file, namespace)
            right_dicts = get_namespace_data(right_file, namespace)

            
            # TODO if we handle multiple messages in a grib file
            # then we need to access the right one here. for now, we look at
            # the first (if multiple)

            left_dict = OrderedDict()
            right_dict = OrderedDict()
            if active_message < len(left_dicts):
                left_dict = left_dicts[active_message]
            if active_message < len(right_dicts):
                right_dict = right_dicts[active_message]

            equal_string = "Identical"

            centre_dict = OrderedDict()   
            for k in left_dict.keys():            
                if k in right_dict.keys():
                    try:
                        if type(right_dict[k]) == FloatType:
                            centre_dict[k] = right_dict[k]-left_dict[k]
                        else:
                            if right_dict[k] != left_dict[k]:
                                centre_dict[k] = "Different"
                            else:
                                centre_dict[k] = equal_string
                    except:
                        centre_dict[k] = "N/A"


            headers = ["key", "value"]
            diff_label = "%s - %s" % (right_label.title(), left_label.title())

            # difference values of the following values will be un-highlighted
            equalvals = [0, equal_string]

            value_keys = left_dict.keys()

            # list of values that we equate as "Equal"
            return render_to_response("viewer/compare_tables.html",
                                        locals(),
                                      context_instance=RequestContext(request))

        return HttpResponse("An error occurred while accessing data")


def serve_grib_compare(request):

    if request.is_ajax():

        page_data = page_dict(request)
            
        #style = request.POST.get("grib_dump_style")
        grib_compare_style = page_data.get("grib_compare_style", None)

        flag_dict={"standard": "", "verbose": "-v",  "values": "-cvalues"}
        flag = flag_dict.get(grib_compare_style, "")


        folder, out_dict = get_test_data(request, page_data, "Output")

        labels = []
        files = []

        for key in out_dict:

            # don't want the comparison file for this view
            if key.upper() == "COMPARISON":
                continue

            output_files = out_dict[key]['data']
            # we expect only one of each type but in case there are
            # more than one we handle that (eventually)
            for filedict in output_files:
                file_type=filedict.get("Type", "Missing")
                if file_type.upper() == "GRIB":
                    labels.append(key)
                    if "File" in filedict.keys():
                        files.append(filedict.get("File"))

        if len(labels) == 2 and len(files) == 2:

            label = "Comparing %s with %s" % (labels[0], labels[1])
            left_label=labels[0]
            right_label=labels[1]
            left_file = files[0]
            right_file = files[1]

            # get some grib_ls data into an object for display
            cmd = "grib_compare -Rvalues=0.00 -f %s %s %s" % (flag, left_file, right_file)
            rtn_data = run_command(cmd)
            data = "Command:\r\n%s\r\n\r\nOutput:\r\n%s" % (cmd, rtn_data)
            

            return render_to_response("viewer/single.html",
                                      locals(),
                                      context_instance=RequestContext(request))

        return HttpResponse("An error occurred while accessing data")

def serve_grib_dump(request):

    if request.is_ajax():

        page_data = page_dict(request)
            
        #style = request.POST.get("grib_dump_style")
        grib_dump_style = page_data.get("grib_dump_style", "standard")

        flag_dict={"standard": "", "octet": "-O", "debug": "-D" }
        flag = flag_dict.get(grib_dump_style, "")


        folder, out_dict = get_test_data(request, page_data, "Output")

        labels = []
        files = []

        for key in out_dict:

            # don't want the comparison file for this view
            if key.upper() == "COMPARISON":
                continue

            output_files = out_dict[key]['data']
            # we expect only one of each type but in case there are
            # more than one we handle that (eventually)
            for filedict in output_files:
                file_type=filedict.get("Type", "Missing")
                if file_type.upper() == "GRIB":
                    labels.append(key)
                    if "File" in filedict.keys():
                        files.append(filedict.get("File"))

        if len(labels) == 2 and len(files) == 2:

            left_label=labels[0]
            right_label=labels[1]
            left_file = files[0]
            right_file = files[1]

            # get some grib_ls data into an object for display
            left_data = run_command("grib_dump %s %s" % (flag, left_file))
            right_data = run_command("grib_dump %s %s" % (flag, right_file))
            

            return render_to_response("viewer/side_by_side.html",
                                      locals(),
                                      context_instance=RequestContext(request))

        return HttpResponse("An error occurred while accessing data")



def spawn_metview(request):
    """
    serves arbitary grib through django without loading into
    memory first
    """

    thefile = request.POST.get("f", None)
    if thefile != None:
        
        if not os.path.exists(thefile):
            return HttpResponse("File not found")

        
        folder, filename = os.path.split(thefile)
        # check it is grib file
        name, ext = os.path.splitext(filename)
        if ext.upper() not in (".GRIB", ".GRB"):
            return HttpResponse("Only GRIB files supported *.grib or *.grb")

        
        #os.system("metview4 -e grib %s &" % thefile)

        response = HttpResponse(thefile, mimetype='application/metview4')
        return response
        return HttpResponse("OK")

    return HttpResponse("Bad request")

def serve_grib_download(request):
    """
    serves arbitary grib through django without loading into
    memory first
    """

    thefile = request.GET.get("f", None)
    #mime = "application/octet-stream"
    mime = "application/x-grib"

    if thefile != None:
        
        if not os.path.exists(thefile):
            return HttpResponse("File not found")


        folder, filename = os.path.split(thefile)
        # check it is grib file
        name, ext = os.path.splitext(filename)
        if ext.upper() not in (".GRIB", ".GRB"):
            return HttpResponse("Only GRIB files supported *.grib or *.grb")

        wrapper = FileWrapper(file(thefile))
        response = HttpResponse(wrapper, content_type=mime)
        response['Content-Length'] = os.path.getsize(thefile)
        response['Content-Disposition'] = "attachment; filename=%s" % filename
        return response
    
    return HttpResponse("No filename specified")



def serve_image(request):

    grib = request.POST.get("f", "Missing")
        
    w = request.POST.get("w", "800")
    h = request.POST.get("h", "400")
    msg = request.POST.get("m", 0)

    try:
        one_indexed_msg = int(msg) + 1
    except:
        one_indexed_msg = 1

    projection = request.POST.get("p", "global")

    contour=request.POST.get("c", "off" )
    shading=request.POST.get("s", "off")
    legend=request.POST.get("l", "off")
    labels = request.POST.get("lb", "off")
    values=request.POST.get("v", "off")
    value_frequency=request.POST.get("fval", "30")

    _diff=request.POST.get("diff", "0")
    is_diff=_diff!="0"
    

    diff_type = request.POST.get("diff_type", None)

    diff_lo = request.POST.get("diff_lo", None)
    diff_hi = request.POST.get("diff_hi", None)

    
    ## generate unique name using uuid to protect against multiple users
    # overwriting each other
    target = "%s/%s_%s.png" % ( settings.STATIC_ROOT, uuid.uuid4(), os.path.basename(grib) )
    filename = ""

    try:

        params = run_command("grib_get -P shortName %s" % grib).strip()
        # Here guard against multiple messages. Eventually will have an index
        # here to access the correct one. For now, take the first
        param_list = params.split("\n")
        # if params was anything at all then it now has length 1 or more    
        grids = run_command("grib_get -P gridType %s" % grib).strip()
        grid_list = grids.split("\n")
    
        
        #if grid_list[0] == "sh":
        #    raise Exception("Cannot view spherical grids")
        

        try: 
            msg = int(msg)
        except:
            msg = 0

        p = param_list[msg]
        
            
        # Special handling of diff images
        # (except for wind cases)
        if is_diff:
            if compare_strings(diff_type, "MISSING DATA"):
                p = "missing"
            else:
                p = "diff"
        
        # Special handling for wind components

        test_list = list(set(param_list))

        if len(test_list) > 1:
            # we have more than one parameter
            if is_wind(test_list):
                #if "u" in test_list and "v" in test_list:
                p = "wind"


        mag_file = "%s/magjson/%s.json" % (settings.STATIC_ROOT, p)
        if not os.path.exists(mag_file):
            mag_file = "%s/magjson/default.json" % (settings.STATIC_ROOT)

        magdata = json.loads(open(mag_file).read())

        projfile = "%s/magjson/projection/%s.json" % ( settings.STATIC_ROOT, projection  )
        projdata = json.loads( open(projfile).read())                             
        # copy projection information over

        for pr in projdata['data']:
            magdata['page']['map'][pr] = projdata['data'][pr]
        

        tmpname = uuid.uuid4()
        filename="%s/%s.json" % (settings.STATIC_ROOT, tmpname)

        if os.path.exists(filename):
            os.remove(filename)

        f = open(filename, "w")
        f.write(json.dumps(magdata))
        f.close()
 

        scale=1.0
        if is_diff:
            # the difference grib files are scaled by 1e6 as otherwise
            # the resolution of the grib file makes all values zero
            scale=DIFF_FILE_SCALE

        cmd = "%smagjson %s -data=%s -width=%s -height=%s -path=%s -proj=%s \
              -contour=%s -shade=%s -legend=%s -label=%s -message=%s \
              -scale=%s -values=%s -fval=%s" % (
                                               settings.MAGJSON_PATH,
                                               filename, 
                                               grib, 
                                               w,h, 
                                               target, 
                                               projection,
                                               contour,
                                               shading, 
                                               legend,labels,
                                               one_indexed_msg,
                                               scale, 
                                               values,
                                               value_frequency)
        if diff_lo:
            cmd += " -min=%s" % diff_lo

        if diff_hi:
            cmd += " -max=%s" % diff_hi

        res = run_command(cmd)
        # get rid of the whitespace
        tmpfile="%s.png" % tmpname
        run_command("mv %s %s" % (target, tmpfile))
        run_command("convert -trim %s %s" % (tmpfile, target))
        image_data = open("%s" % target, "rb").read()
        os.remove(tmpfile)

    except:
        msg = "Image could not be generated" 
        cmd = "convert -background lightblue -gravity center -size %sx%s -fill blue -pointsize 48 label:'%s' %s" % (w,h,msg,target)
        res = run_command(cmd)
        image_data = open("%s" % target, "rb").read()
        
    im = Image.open(target)
    w, h = im.size
    
    try:
        for f in [target, filename]:
            if os.path.exists(f): 
                os.remove(f)
    except:
        pass
    
    import base64
    image_data64 = base64.b64encode(image_data)

    rtn_dict = {"w": w, "h": h, "img64": image_data64}
    
    
    return HttpResponse(json.dumps(rtn_dict))
    


def serve_images(request):

    if request.is_ajax():

        page_data = page_dict(request)
        selected_image_layout = page_data.get("selected_image_layout", None)

        data_diff_type = page_data.get("data_diff_type", None)

        folder, out_dict = get_test_data(request, page_data, "Output")

        labels = []
        #files = []
        #scripts = []

        diff_label = "Missing"
        diff_image = ""   # can add a "missing" image to our media url

           
        active_message = page_data.get("active_message", 0)
        one_indexed_msg = int(active_message)+1

        width=800
        height=400

        projection= page_data.get("projection", "global")
        resolution = page_data.get("resolution", "low")
        value_frequency = 5
        if resolution=="high":
            width = width * 2
            height = height * 2
            value_frequency=2


        contour=page_data.get("contour", "off") 
        shading=page_data.get("shading", "off") 
        legend=page_data.get("legend", "off") 
        contour_labels=page_data.get("contour_labels", "off") 
        values=page_data.get("values", "off")
        

        data_diff_type=page_data.get("data_diff_type", None)

        hide_tolerance_area = compare_strings(data_diff_type, "MISSING DATA")

        grib_files = []
        minmax_list = []

        comparison_grib = ""
    

        for key in out_dict:
            # look for grib files
            if "data" in out_dict[key].keys():
                output_files = out_dict[key]['data']
                for filedict in output_files:
                    file_type=filedict.get("Type", "Missing")
                    file_label=filedict.get("Label", "Missing")
                    if file_type.upper() == "GRIB":
                        if "File" in filedict.keys():
                            this_file = filedict.get("File")
                            if key.upper() == "COMPARISON":
                                if file_label==data_diff_type:
                                    comparison_grib = this_file
                                    diff_label = key
                                    try:
                                        diff_minmax= [DIFF_FILE_SCALE * float(get_grib_key(this_file, one_indexed_msg, k)) for k in ["min", "max"]]
                                    except:
                                        diff_minmax=[0.0,0.0]
                            else:
                                grib_files.append(this_file)
                                labels.append(key)

                                try:
                                    minval = get_grib_key(this_file, one_indexed_msg, "min")
                                    maxval = get_grib_key(this_file, one_indexed_msg, "max")
                                    minmax_list.append((minval, maxval))
                                except:
                                    minmax_list.append([-10000.0,10000.0])


        if len(grib_files) == 2:

            left_grib = grib_files[0]
            right_grib = grib_files[1]
        
            left_minmax = minmax_list[0]
            right_minmax = minmax_list[1]

            left_label=labels[0]
            right_label=labels[1]



            return render_to_response("viewer/images.html",
                                    locals(),
                                  context_instance=RequestContext(request))


        return HttpResponse("An error occurred while accessing data")
