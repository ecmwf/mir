# my_filters.py
# Some custom filters for dictionary lookup.
from django.template.defaultfilters import register

@register.filter(name='lookup')
def lookup(dict, key):
    if key in dict:
        return dict[key]
    return ''

@register.filter(name='contains')
def contains(list, val):
    if val in list:
        return True
    return False

@register.filter(name='titleify')
def titleify(value):
    # remove underscores, replacing with spaces
    # and capitalise first letters of results
    newstr = value.replace("_", " ")
    return " ".join([w[0].upper() + w[1:] for w in newstr.split(" ")])


@register.filter(name='addone')
def addone(value):
    # turns zero-indexed into one-indexed for display only
    return value+1

def sliced(value, upto):
    """Use: {% for a in mylist|sliced:z %}"""
    try:
        return value[0:upto]
    except (ValueError, TypeError):
        return value # Fail silently.
