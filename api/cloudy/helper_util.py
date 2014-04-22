import random
from datetime import datetime
from uuid import getnode

size = 2
api_dict = {
    "F237E8FB2657FFFE5878AC972CA67":"dev",
}

def gen_new_graph():
    graph = ""
    for i in xrange( size*size ):
        graph += str( int( round( random.random(),0 ) ) )
    return graph

def gen_id():
    d = datetime.now()
    sec_since_epoc = str(int((d-d.utcfromtimestamp(0)).total_seconds()))
    mac = str( getnode() )
    return int(mac[0:6]+sec_since_epoc)

def authorized_api_key( key ):
    return api_dict.has_key( key )



