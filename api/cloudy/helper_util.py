import random
from datetime import datetime
from uuid import getnode

# Size of the matrix to be generated
size = 100

# Keys for master system
api_dict = {
    "F237E8FB2657FFFE5878AC972CA67":"dev",
}
# Keys for master system reverse lookup
get_master_api_key = {
    "dev":"F237E8FB2657FFFE5878AC972CA67",
}

# Bank (host, port)
bank_host = [
    ('richcoin.cs.ucsb.edu', '8243'),
]

# Keys for banking system
bank_api_key = {
    "dev":'XpAEFTthbQGnNgp7ZXfRmFm1HIca',
    "prod":" qb9gzaZ2_2vT5hWgNyENIZGwgQ4a",
}

def gen_new_graph():
    graph = ""
    for i in xrange( size*size ):
        graph += str( int( round( random.random(),0 ) ) )
    return graph

# Generate a unique ID using MAC-address and time
# to avoid two objects created at the same time on different master nodes
def gen_id():
    d = datetime.now()
    sec_since_epoc = str(int((d-d.utcfromtimestamp(0)).total_seconds()))
# Getting the mac address for the computer
    mac = str( getnode() )
# Combine the 6 most significant numbers of the mac address and the time
    return int(mac[0:6]+sec_since_epoc)

def authorized_api_key( key ):
    return api_dict.has_key( key )

def get_bank_connection_url():
    bh = bank_host[0]
    return "%s:%s"%( bh[0], bh[1] )
    
def get_bank_key( app_name ):
    return bank_api_key[ app_name ] if bank_api_key.has_key( app_name ) else None
