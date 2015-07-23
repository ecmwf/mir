#!/usr/bin/env python
'''
    Submits mars client testing to ecflow

    The following arguments may be specified:

    -c, --config-file=FILEPATH   Configuration file to use other than default.
    -h, --help                   Shows this help.

'''

import os, errno
import ecflow
import shutil
import sys
import time
import getopt
import pprint
import datetime

import combinations
import itertools 

def clear_path(path):
    
    print "clear_path: Removing folder %s" % path
    # wipes existing path
    try:
        if (os.path.isdir(path)):            
            shutil.rmtree(path)
            print "Folder %s removed" % path
    except RuntimeError, e:
        print "Couldn't remove existing folder %s. Exiting." % path
        sys.exit(1)

def server_exists(client):
    try:
        cl.ping()
        return True
    except RuntimeError, e:
        print "Ping of server %s:%s failed: %s" % (server_hostname,
                                               str(server_port),
                                               str(e))
    return False

def mkdir_p(path):
    # mkdir -p functionality
    try:
        print "making dir %s" % path
        os.makedirs(path)
    except OSError as exc: # Python >2.5
        if exc.errno == errno.EEXIST:
            print "OK path %s already exists" % path
            pass
        else: raise


def mangle_name_for_ecf(name):
    return name.replace("+", "_plus")

def parse_config_file(filename):
    config_dict={}

    lines = open(filename, "r")
    for line in lines:
        line=line.strip()

        if len(line) == 0 or line[0]=="#":
            # line is comment or blank            
            continue

        tokens=line.split("=")
        if len(tokens) == 2:
            config_dict[tokens[0]]=tokens[1].strip()
        else:
            print "Unable to parse line %s" % line

    return config_dict

# Get settings from the path to the executed file
config_file="%s/default.settings" % sys.path[0]
print "Using config file %s" % config_file

# Whether to ignore warnings and plough on
force_submission=False


try:
    opts, args = getopt.getopt(sys.argv[1:], "hfc:", ["help", "force", "config-file="])
except getopt.error, msg:
    print msg
    print "for help use --help"
    sys.exit(2)

for o, a in opts:
    if o in ("-h", "--help"):
        print __doc__
        sys.exit(0)
    elif o in ("-c", "--config-file"):
        config_file=a
    elif o in ("-f", "--force"):
        force_submission=True
   

config_dict=parse_config_file(config_file)

# work out the suite name from the config file
# so if the config file is blahblah.settings then we 
# run with a suite name of "blahblah"
config_file_base=os.path.basename(config_file)
suite_name, _ =os.path.splitext(config_file_base) 


working_folder = "."
if "ECF_HOME" in config_dict.keys():

    working_folder = config_dict["ECF_HOME"]    
    mkdir_p(working_folder)
    print "ensuring folder %s exists" % working_folder


defs=ecflow.Defs()
suite=defs.add_suite(suite_name)

folder_path="%s/%s" % (working_folder, suite_name)
clear_path(folder_path)
mkdir_p(folder_path)

suite.add_variable("ECF_INCLUDE", working_folder) 
suite.add_variable("ECF_HOME",    working_folder)
suite.add_variable("ECF_FILES",   working_folder)


# copy minimum files over
# if not running locally
if os.path.realpath(".") != os.path.realpath(working_folder):
    print "Copying required files to remote working folder..."

    # these local files need to be copied to the remove working folder
    files=["error_codes.sh", "head.h", "tail.h", "utils.sh", "xml.sh",
           "get_source_parameters.sh",] 
    # as do these folders
    folders=["checks", ]

    for f in files:
        remote_file="%s/%s" % (working_folder, f)
        print "File: %s" % remote_file
        if os.path.exists(remote_file):
            os.remove(remote_file)
        shutil.copyfile("./%s" % f, remote_file)

    for fd in folders:
        remote_folder = "%s/%s" % (working_folder, fd)
        print "Folder: %s" % remote_folder
        clear_path(remote_folder)
        shutil.copytree("./%s" % fd, remote_folder)

    # copy over the scripts that switch interpolation
    for interp in combinations.comparisons:
        f = "go_%s.sh" % interp
         
        remote_file = "%s/../%s" % (working_folder, f)
        if os.path.exists(remote_file):
            os.remove(remote_file)
        # copy the local stub file to the remote file
        shutil.copyfile("go_test.sh", remote_file)

    print "All files copied."


test_counter = 0

client_list = ["MARS"]

output_root_folder="%s/output" % working_folder
print "*** output_root_folder is %s" % output_root_folder
mkdir_p(output_root_folder)
output_folder='%s' % output_root_folder

run_date=datetime.datetime.now().strftime('%Y%m%d')
suite.add_variable("RUN_DATE", run_date)

suite.add_variable("OUTPUT_FOLDER", output_folder)

run_comparison = 1 if combinations.run_comparison else 0
suite.add_variable("RUN_COMPARISON", run_comparison)

dummy_run = 1 if combinations.dummy_run else 0
suite.add_variable("DUMMY_RUN", dummy_run)

for client in client_list:     

    client_path="%s/%s" % (folder_path, client)

    clear_path(client_path)
    mkdir_p(client_path)

    cfamily=suite.add_family(mangle_name_for_ecf(client))
    cfamily.add_variable("CLIENT", client)
    cfamily.add_limit("simultaneous_platforms", 10)
    cfamily.add_inlimit("simultaneous_platforms")

    for plat, machines in combinations.platform_dict.items():

        root_path="%s/%s" % (client_path, plat)

        clear_path(root_path) 
        mkdir_p(root_path)
        
        pfamily=cfamily.add_family(mangle_name_for_ecf(plat))
        pfamily.add_variable("PLATFORM", plat)
        
        pfamily.add_limit("simultaneous_machines", 10)
        pfamily.add_inlimit("simultaneous_machines")


        for m in machines:
            
            mfamily = pfamily.add_family(mangle_name_for_ecf(m))
            mfamily.add_variable("MACHINE", m)

            mach_root_path = "%s/%s" % (root_path, m)
            mkdir_p(mach_root_path)
        
            for grid, tgrids in combinations.grid_dict.items():

                gfamily=mfamily.add_family(mangle_name_for_ecf(grid))
                gfamily.add_variable("SOURCE_GRID", grid)
                grid_path="%s/%s" % (mach_root_path, grid)
                mkdir_p("%s" % grid_path)

                # get the test names for this source grid
                source_requests = combinations.src_requests[grid]
                for request_name in source_requests.keys():
                    
                    request = source_requests[request_name]

                    tfamily=gfamily.add_family(mangle_name_for_ecf(request_name))
                    tfamily.add_variable("SOURCE_REQUEST", request)
                    tfamily.add_variable("SOURCE_TAG", request_name)

                    #enable massif / valgrind for this src grid
                    memcheck = 1 if grid in combinations.memcheck_grids else 0

                    tfamily.add_variable("RUN_MEMORY_CHECKS", memcheck)

                    test_path="%s/%s" % (grid_path, request_name)
                    mkdir_p(test_path)

                    # we limit tasks below this level
                    tfamily.add_limit("subtasks", 1)
                    tfamily.add_inlimit("subtasks")


                    # add a fetch here
                    fetch_task_name="fetch_data"
                    fetch_task = tfamily.add_task(fetch_task_name)
                    taskfile="%s.ecf" % fetch_task_name
                    fromstr="./%s" % taskfile
                    tostr="%s/%s" % (test_path, taskfile)
                    print "copying file from %s to %s" % (fromstr, tostr)
                    shutil.copyfile(fromstr, tostr)


                    for tgrid in tgrids:

                        tgfamily=tfamily.add_family(tgrid)
                        tgfamily.add_variable("TARGET_GRID", tgrid)
                        tgfamily.add_trigger("%s == complete" % fetch_task_name)

                        # find the target request stubs for this target grid
                        tg_path = "%s/%s" % (test_path, tgrid)
                        mkdir_p(tg_path)

                        tcount = 0
                        vadjust = dict(combinations.adjust[tgrid].items() + combinations.adjust["all"].items())
                        allkeys = vadjust.keys()
                        allvecs = vadjust.values()
                        req_folder = "%s/requests" % working_folder
                        mkdir_p(req_folder)

                        print "Using request folder %s" % req_folder

                        reqs_file = "%s/%s_%s_%s.reqs" % (req_folder, grid, request_name, tgrid)
                        with open(reqs_file, "w") as the_file:
                            for xx in itertools.product(*allvecs):
                                x = list(xx)
                                assert(len(x) == len(allkeys))
                                # Hack alert:
                                # look for area being global. if so, disable
                                # frame as this is something we don't want to
                                # have to simulate (we will always be able to
                                # provide frames so not worth testing whether
                                # we can do it when we are able to like emos
                                # has to)
                                try:
                                    area_index=allkeys.index("area")
                                    frame_index=allkeys.index("frame")
                                    if (x[area_index] == "off"):
                                        print "NB: area is off, so changing frame value from %s to off" % x[frame_index]
                                        x[frame_index] = "off"

                                
                                except ValueError, e:
                                    # ignore fact that area or frame weren't
                                    # in there after all
                                    pass
                                
                                the_file.write("%s\n" % "".join(["%s=%s," % (allkeys[p], x[p]) for p in   range(0,len(allkeys))]))
                                tcount += 1
                        print "There are %d requests for %s -> %s" % (tcount,
                                                                      grid,
                                                                      tgrid)

                        # now we add a test to do a loop over this grid to
                        # grid combination
                        test_name = "run_%d_requests" % tcount
                        dfamily = tgfamily.add_family(test_name)
                        dfamily.add_variable("REQUESTS_FILE", reqs_file)
                        dest_path="%s/%s" % (tg_path, test_name)
                        mkdir_p(dest_path)

                        for icount, interp in enumerate(combinations.comparisons):
                            dfamily.add_variable("INTERPOLATOR%d" % icount, interp)
                        for icount, mars_path in enumerate(combinations.mars_folders):
                            dfamily.add_variable("MARS_PATH%d" % icount, mars_path)
                            print "MARS_PATH %d = %s" % (icount, mars_path)
                        for icount, mars_cmd in enumerate(combinations.mars_commands):
                            dfamily.add_variable("MARS_CMD%d" % icount, mars_cmd)
                            print "MARS_CMD %d = %s" % (icount, mars_cmd)
                        
                    


                        

                        task="run"
                        print "adding task %s" % task
                        taskfile="%s.ecf" % task
                        fromstr="./%s" % taskfile
                        tostr="%s/%s" % (dest_path, taskfile)
                        shutil.copyfile(fromstr, tostr)
                        
                         
                        task_obj = dfamily.add_task(task)
                        test_counter += tcount
        
        
# All individual test runs now set up
#
# Add task to create an index file to all the individual xml files
# and create a summary web page.

summary_task=suite.add_task("summary")
shutil.copyfile("./summary.ecf", "%s/summary.ecf" % working_folder)

for count, client in enumerate(client_list):
    if count == 0:
        summary_task.add_part_trigger("%s == complete" % client)
    else:
        # AND the new condition
        summary_task.add_part_trigger("%s == complete" % client, True)


# Create a test .def file to see whether 
# we have set things up correctly
# (this should fail if we haven't)

print "Creating def file for testing..."                
defs.save_as_defs("check.def")

# now use the client to get things running
cl = ecflow.Client()
server_hostname=config_dict.get("ECFLOW_SERVER_HOSTNAME", "localhost")
server_port=config_dict.get("ECFLOW_SERVER_PORT", 4000)
cl.set_host_port(server_hostname, str(server_port))

if server_exists(cl) == False:
    print "Server not running on %s:%s. Exiting." % (server_hostname,
                                                     str(server_port))
    sys.exit(1)

try:

    # get a copy of the server defs on the client
    cl.sync_local()
    
    current_defs=cl.get_defs()
    if current_defs != None:
        current_suite=cl.get_defs().find_suite(suite_name)

        if force_submission == False and current_suite != None:
            print "Suite name %s already exists. Re-run using --force (-f) option.  " % suite_name
            sys.exit(1)


    cl.load(defs, force_submission)
    print "Loaded suite definition."
except RuntimeError, e:
    cl.load(defs)
    print "Load of suite definition failed: %s" % str(e)
    sys.exit(1)


try:
    cl.begin_suite(suite_name)
    print "Suite started successfully. %s tests submitted" % test_counter
except RuntimeError, e:
    print "Suite did not start: %s" % str(e)
    sys.exit(1)

