import ecregrid_swig as _internal
from functools import wraps
import gribapi

def __boolify(fn):
    def wrapped(*args):
        return fn(*args) == 0
    return wrapped

# function arguments type checking decorator
# inspired by http://code.activestate.com/recipes/454322-type-checking-decorator/
# modified to support multiple allowed types and all types in the same decorator call
def __require(**_params_):
    def check_types(_func_, _params_ = _params_):
        @wraps(_func_)
        def modified(*args, **kw):
            arg_names = _func_.func_code.co_varnames
            #argnames, varargs, kwargs, defaults = inspect.getargspec(_func_)
            kw.update(zip(arg_names, args))
            for name, allowed_types in _params_.iteritems():
                param = kw[name]
                if type(allowed_types) == types.TypeType:
                    allowed_types = (allowed_types,)
                assert type(param) in allowed_types,  \
                    "Parameter '%s' should be type %s" % (name, " or ".join([t.__name__ for t in allowed_types]))
            return _func_(**kw)
        return modified
    return check_types

class __InternalError(Exception):
    def __init__(self, code):
        self.code = code
    def __str__(self):
        return repr(self.code)

# Checks validity of return object
def __raise_if_error(obj):
    if isinstance(obj, (int, long)):
        if obj != 0:
            raise __InternalError(obj)

# Checks validity of message type
def __check_message(obj):
    if not isinstance(obj, str):
        raise __InternalError(1)

def __check_min_list_length(obj, nvals):
    if isinstance(obj, list):
        if len(obj) >= nvals:
            return
    raise __InternalError(1)

#
# Public API below
#

def get_version():
    """
    @brief Get the api version.

    Returns the version of the api as a string in the format "major.minor.revision".
    """
    div = lambda v,d: (v/d,v%d)
    v = _internal.ecregrid_get_version()
    v,revision = div(v,100)
    v,minor = div(v,100)
    major = v

    return "%d.%d.%d" % (major,minor,revision)


def get_field_description(field_type):
    try:
        rtn = _internal.ecregrid_get_field_description(field_type) 
        __raise_if_error(rtn)
        __check_min_list_length(rtn, 2)
        __raise_if_error(rtn[1])
        index = _internal._add_field_description(rtn[0])
        return index

    except __InternalError as e:
        print "ecRegrid returned %s" % e.code
        return None 

def get_field_description_general():

    try:
        rtn = _internal.ecregrid_get_field_description_general() 
        __raise_if_error(rtn)
        __check_min_list_length(rtn, 2)
        __raise_if_error(rtn[1])
        index = _internal._add_field_description(rtn[0])
        return index
        
    except __InternalError as e:
        print "ecRegrid returned %s" % e.code
        return None 

def field_description_destroy(index):
    fd = _internal._remove_field_description(index)
    if fd:
        _internal.ecregrid_field_description_destroy(fd)

def get_field_description_from_grib_handle(gh):
    # we are passed a gribapi/python-style "handle" which is actually an
    # integer. we need to convert this to an actual grib_handle that
    # wraps the C object of the same name
   
    try:

        # do this by getting message from this handle
        msg=gribapi.grib_get_message(gh)
        __check_message(msg)

        actual_handle=_internal._ecregrid_util_local_grib_handle_from_message(msg, len(msg))
        rtn = _internal.ecregrid_get_field_description_from_grib_handle(actual_handle)
        __raise_if_error(rtn)
        __check_min_list_length(rtn, 2)
        __raise_if_error(rtn[1])

        index = _internal._add_field_description(rtn[0])
        return index

    except __InternalError as e:
        print "ecRegrid returned %s" % e.code
        return None 
    


def process_grib_handle_to_grib_handle(gh, fd_handle):

    try:
        fd = _internal._get_field_description_from_handle(fd_handle)
        msg=gribapi.grib_get_message(gh)
        __check_message(msg)

        actual_input_handle=_internal._ecregrid_util_local_grib_handle_from_message(msg, len(msg))        
        rtn = _internal.ecregrid_process_grib_handle_to_grib_handle(actual_input_handle, fd)
        if (rtn == 0 and isinstance(rtn, int)):
            # in this case we need to return clone of input handle
            return gribapi.grib_clone(gh)

        __raise_if_error(rtn)
        __check_min_list_length(rtn, 2)
        __raise_if_error(rtn[1])
        actual_output_handle=rtn[0]

        err, out_msg=_internal._ecregrid_util_message_from_local_grib_handle(actual_output_handle)
        __raise_if_error(err)
        __check_message(out_msg)

        return gribapi.grib_new_from_message(out_msg)

    except __InternalError as e:
        print "ecRegrid returned %s" % e.code                
        return None


def one_point(gh, lat, lon):

    try:
        msg=gribapi.grib_get_message(gh)
        __check_message(msg)

        actual_input_handle=_internal._ecregrid_util_local_grib_handle_from_message(msg, len(msg))
        rtn = _internal.ecregrid_one_point(actual_input_handle, lat, lon)
        __raise_if_error(rtn)
        __check_min_list_length(rtn, 2)
        __raise_if_error(rtn[0])
        return rtn[1]

    except __InternalError as e:
        print "ecRegrid returned %s" % e.code                
        return None


def process_grib_handle_to_unpacked(gh, fd_handle):
    try:
        fd = _internal._get_field_description_from_handle(fd_handle)
        msg=gribapi.grib_get_message(gh)
        __check_message(msg)

        actual_input_handle=_internal._ecregrid_util_local_grib_handle_from_message(msg, len(msg))
        rtn = _internal.ecregrid_process_grib_handle_to_unpacked(actual_input_handle, fd)
        
        __raise_if_error(rtn)
        __check_min_list_length(rtn, 3)
        __raise_if_error(rtn[2])

        datalen = rtn[1]
        data = _internal.doublearray_frompointer(rtn[0])
        return [data[i] for i in xrange(datalen)]
    
    except __InternalError as e:
        print "ecRegrid returned %s" % e.code                
        return None


def process_unpacked_to_grib_handle(fd_handle_in, data_list, fd_handle_out):
    
    try:
        fd_in = _internal._get_field_description_from_handle(fd_handle_in)
        fd_out = _internal._get_field_description_from_handle(fd_handle_out)

        nval = len(data_list)
        pdata = _internal.doublearray(nval)

        for i in xrange(nval):
            pdata[i]=data_list[i]

        rtn = _internal.ecregrid_process_unpacked_to_grib_handle(fd_in, pdata, nval, fd_out);
        __raise_if_error(rtn)
        __check_min_list_length(rtn, 2)
        __raise_if_error(rtn[1])
            
        actual_output_handle=rtn[0]
        err, out_msg=_internal._ecregrid_util_message_from_local_grib_handle(actual_output_handle)
        __raise_if_error(err)
        __check_message(out_msg)
                        
        return gribapi.grib_new_from_message(out_msg)

        raise __InternalError(1)
    except __InternalError as e:
        print "ecRegrid returned %s" % e.code

def process_unpacked_to_unpacked(fd_handle_in, data_list, fd_handle_out):

    try:

        fd_in = _internal._get_field_description_from_handle(fd_handle_in)
        fd_out = _internal._get_field_description_from_handle(fd_handle_out)

        nval = len(data_list)
        pdata_in = _internal.doublearray(nval)
        for i in xrange(nval):
            pdata_in[i]=data_list[i]

        rtn = _internal.ecregrid_process_unpacked_to_unpacked(fd_in, pdata_in, nval, fd_out)
        __raise_if_error(rtn)
        __check_min_list_length(rtn, 3) 
        __raise_if_error(rtn[2])

        out_datalen = rtn[1]
        out_data = _internal.doublearray_frompointer(rtn[0])
        return [out_data[i] for i in xrange(out_datalen)]
    
    except __InternalError as e:
        print "ecRegrid returned %s" % e.code


def process_read_from_file_to_grib_handle(fd_handle_in, filename, fd_handle_out):
    try:
        fd_in = _internal._get_field_description_from_handle(fd_handle_in)
        fd_out = _internal._get_field_description_from_handle(fd_handle_out)

        rtn = _internal.ecregrid_process_read_from_file_to_grib_handle(fd_in, filename, fd_out)
        __raise_if_error(rtn)
        __check_min_list_length(rtn, 2)
        __raise_if_error(rtn[1])

        actual_output_handle=rtn[0]
        err, out_msg=_internal._ecregrid_util_message_from_local_grib_handle(actual_output_handle)
        __raise_if_error(err)
        __check_message(out_msg)

        return gribapi.grib_new_from_message(out_msg)

    except __InternalError as e:
        print "ecRegrid returned %s" % e.code
            

def process_vector_grib_handle_to_grib_handle(ghU, ghV, fd_handle):

    try:

        fd = _internal._get_field_description_from_handle(fd_handle)
        input_messages = [gribapi.grib_get_message(h) for h in (ghU, ghV)]
        for msg in input_messages: __check_message(msg)
        
        actual_handle_u, actual_handle_v = [_internal._ecregrid_util_local_grib_handle_from_message(m, len(m)) for m in input_messages]
        
        rtn = _internal.ecregrid_process_vector_grib_handle_to_grib_handle(actual_handle_u, actual_handle_v, fd)
        __check_min_list_length(rtn, 2)
        __raise_if_error(rtn[1])

        actual_output_handle_vec = rtn[0]

        output_grib_handles = [_internal._ecregrid_util_vector_grib_handle_get_at(actual_output_handle_vec, i) for i in (0, 1)]
        
        output_messages = []
        for actual_handle in output_grib_handles:
            err, out_msg= _internal._ecregrid_util_message_from_local_grib_handle(actual_handle)
            __raise_if_error(err)
            __check_message(out_msg)
            output_messages.append(out_msg)

        return [gribapi.grib_new_from_message(out_msg) for out_msg in output_messages]

    except __InternalError as e:
        print "ecRegrid returned %s" % e.code

@__boolify
def set_area(fd_handle, north, west, south, east):
    fd = _internal._get_field_description_from_handle(fd_handle)
    return _internal.ecregrid_set_area(fd, north, west, south, east)

@__boolify
def set_south_pole(fd_handle, lat, lon):
    fd = _internal._get_field_description_from_handle(fd_handle)
    return _internal.ecregrid_set_south_pole(fd, lat, lon)

@__boolify
def set_increments(fd_handle, we, ns):
    fd = _internal._get_field_description_from_handle(fd_handle)
    return _internal.ecregrid_set_increments(fd, we, ns)

@__boolify
def set_truncation(fd_handle, t):
    fd = _internal._get_field_description_from_handle(fd_handle)
    return _internal.ecregrid_set_truncation(fd, t)

@__boolify
def set_gaussian_number(fd_handle, n):
    fd = _internal._get_field_description_from_handle(fd_handle)
    return _internal.ecregrid_set_gaussian_number(fd, n)

@__boolify
def set_scanning_mode(fd_handle, n):
    fd = _internal._get_field_description_from_handle(fd_handle)
    return _internal.ecregrid_set_scanning_mode(fd, n)

@__boolify
def set_grid_type(fd_handle, name):
    fd = _internal._get_field_description_from_handle(fd_handle)
    return _internal.ecregrid_set_grid_type(fd, name)

@__boolify
def set_reduced_grid_definition(fd_handle, gridDef, size):
    fd = _internal._get_field_description_from_handle(fd_handle)
    return _internal.ecregrid_set_reduced_grid_definition(fd, gridDef, size)

@__boolify
def set_is_reduced(fd_handle, n):
    fd = _internal._get_field_description_from_handle(fd_handle)
    return _internal.ecregrid_set_is_reduced(fd, n)

@__boolify
def set_frame_number(fd_handle, n):
    fd = _internal._get_field_description_from_handle(fd_handle)
    return _internal.ecregrid_set_frame_number(fd, n)

@__boolify
def set_bitmap_file(fd_handle, name):
    fd = _internal._get_field_description_from_handle(fd_handle)
    return _internal.ecregrid_set_bitmap_file(fd, name)

@__boolify
def set_list_of_points_file(fd_handle, name):
    fd = _internal._get_field_description_from_handle(fd_handle)
    return _internal.ecregrid_set_list_of_points_file(fd, name)

@__boolify
def set_file_type(fd_handle, name):
    fd = _internal._get_field_description_from_handle(fd_handle)
    return _internal.ecregrid_set_file_type(fd, name)

@__boolify
def set_list_of_points_file_type(fd_handle, typ):
    fd = _internal._get_field_description_from_handle(fd_handle)
    return _internal.ecregrid_set_list_of_points_file_type(fd, typ)

@__boolify
def set_interpolation_method(fd_handle, method):
    fd = _internal._get_field_description_from_handle(fd_handle)
    return _internal.ecregrid_set_interpolation_method(fd, method)

@__boolify
def set_lsm_method(fd_handle, method):
    fd = _internal._get_field_description_from_handle(fd_handle)
    return _internal.ecregrid_set_lsm_method(fd, method)

@__boolify
def set_number_of_points(fd_handle, ns, we):
    fd = _internal._get_field_description_from_handle(fd_handle)
    return _internal.ecregrid_set_number_of_points(fd, ns, we)

@__boolify
def number_of_nearest_points(fd_handle, n):
    fd = _internal._get_field_description_from_handle(fd_handle)
    return _internal.ecregrid_ecregrid_number_of_nearest_points(fd, n)

@__boolify
def set_date(fd_handle, date):
    fd = _internal._get_field_description_from_handle(fd_handle)
    return _internal.ecregrid_set_date(fd, date)
    
@__boolify
def set_time(fd_handle, timeval):
    fd = _internal._get_field_description_from_handle(fd_handle)
    return _internal.ecregrid_set_time(fd, timeval)

@__boolify
def set_edition_number(fd_handle, grib):
    fd = _internal._get_field_description_from_handle(fd_handle)
    return _internal.ecregrid_set_edition_number(fd, grib)

@__boolify
def set_start_step(fd_handle, step):
    fd = _internal._get_field_description_from_handle(fd_handle)
    return _internal.ecregrid_set_start_step(fd, step)

@__boolify
def set_end_step(fd_handle, t):
    fd = _internal._get_field_description_from_handle(fd_handle)
    return _internal.ecregrid_set_end_step(fd, t)

@__boolify
def set_extrapolate_on_pole(fd_handle, typename):
    fd = _internal._get_field_description_from_handle(fd_handle)
    return _internal.ecregrid_set_extrapolate_on_pole(fd, typename)

@__boolify
def set_derived_parameter(fd_handle, paramname):
    fd = _internal._get_field_description_from_handle(fd_handle)
    return _internal.ecregrid_set_derived_parameter(fd, paramname)

@__boolify
def set_missing_value(fd_handle, value):
    fd = _internal._get_field_description_from_handle(fd_handle)
    return _internal.ecregrid_set_missing_value(fd, value)

@__boolify
def set_parameter_id(fd_handle, paramId):
    fd = _internal._get_field_description_from_handle(fd_handle)
    return _internal.ecregrid_set_parameter_id(fd, paramId)

@__boolify
def set_auresol(fd_handle, value):
    fd = _internal._get_field_description_from_handle(fd_handle)
    return _internal.ecregrid_set_auresol(fd, value)


