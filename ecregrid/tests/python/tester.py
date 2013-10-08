import sys, os, errno
sys.path.append("../../python")
import gribapi
import ecregrid

from locations import data_in, data_out, data_ref

def __run_command(cmd, use_ksh=False):
    # runs a shell command and returns output
    print "__run_command %s" % cmd
    
    if use_ksh:
        # enforce ksh
        cmd = "ksh -c \"%s\"" % cmd

    proc = os.popen(cmd)
    data = proc.read()  
    proc.close()
    return data.strip()

def __ensure_output_folder():
    try:
        os.makedirs(data_out)
    except OSError as exc: # Python >2.5
        if exc.errno == errno.EEXIST:
            pass

def run_transformation(infile, outfile, fd, is_wind=False):


    __ensure_output_folder()

    _, ext =  os.path.splitext(outfile)

    fin=open(os.path.join(data_in, infile), "r")
    fout=open(os.path.join(data_out, outfile), "w")

    count=gribapi.grib_count_in_file(fin)

    if is_wind:

        if count != 2:
            return

        hU=gribapi.grib_new_from_file(fin)
        hV=gribapi.grib_new_from_file(fin)

        handles = ecregrid.process_vector_grib_handle_to_grib_handle(hU, hV, fd)

        for h in handles:
            if h:
                gribapi.grib_write(h, fout)        
                gribapi.grib_release(h)

        gribapi.grib_release(hU)
        gribapi.grib_release(hV)
    
    else:

        for i in range(count):

            hin=gribapi.grib_new_from_file(fin)

            if ext == ".txt":
                data = ecregrid.process_grib_handle_to_unpacked(hin, fd)
                for d in data:
                    fout.write("%.6f\n" % d)
            else:
               
                hout= ecregrid.process_grib_handle_to_grib_handle(hin, fd)
                if hout:
                    gribapi.grib_write(hout, fout)
                    gribapi.grib_release(hout)
                else:
                    print "%s: None-type handle returned from API function 'process_grib_handle_to_grib_handle'" % __file__

            gribapi.grib_release(hin)

    fin.close()
    fout.close()

def compare_with_reference(filename):
    return compare_files(os.path.join(data_out, filename), os.path.join(data_ref, filename))

def compare_files(file1, file2):

    files = (file1, file2)
    for f in files:
        if not os.path.exists(f):
            return "File %s does not exist" % f

    # run grib_compare on them if grib, else if txt file do something else
    _, ext = os.path.splitext(file1)
    if ext == ".txt":
        # Current API for python outputs only value rather than "lat lon value"
        # as output by the tool. This is because the tool uses non-api calls
        # to get at these values. Hence we only compare the values themselves
        # and not the lat/lons.
        # Some shell manipulation is required... here we assume there are two spaces between 
        # the numbers, i.e. "lat  lon  value"
        return __run_command("diff --ignore-all-space %s <(cat %s | cut -f 5 -d\' \'  )" % (files), True)
        #return __run_command("diff --ignore-all-space %s %s" % (files))
    else:
        return __run_command("grib_compare -P %s %s" % (files))
                                                
def report_output(output):
    if len(output) == 0:
        print "PASS"
        # do not exit
    else:
        print "FAILED:"
        print output
        # exit fail code
        sys.exit(1)
