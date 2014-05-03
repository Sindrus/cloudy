#!/usr/bin/python
import os
import sys
import httplib
import json
import time
os.environ.setdefault("DJANGO_SETTINGS_MODULE", "cloudy.settings")

from graph.models import Graph
from graph.serializers import MasterGraphSerializer
from masters.models import Master
from rest_framework.parsers import JSONParser
from rest_framework.compat import BytesIO
from rest_framework.exceptions import ParseError
import helper_util

def check_coins_to_validation():
    for g in Graph.objects.filter( is_to_validation=True ):
        p = send( 'GET', '/vault/1.0.0/%s'%g.coin_id )
        g.coin_status = p[ 'status' ]
        if p[ 'status' ] is 1 or p[ 'status' ] is 2:
            g.is_to_validation = False
            g.in_progress = False
            g.error_code = p[ 'reason' ]

        g.save()

def send_to_validation():
    for g in Graph.objects.filter( is_solution=True, is_to_validation=False ):
        if g.coin_status==1 or g.coin_status==2:
            continue

# Not sure how to sync the status of how many coins are currently
# to validation. As two masters are planned, this will make sure
# at most 10 coins are to validation. It may however do the actual
# maximum number of coins being validated be less than 10.
# Better safe than sorry.
# TODO: Find a way to know the global number of coins to validation.
        if( len( Graph.objects.filter( is_to_validation=True ) ) >= 5 ):
            return

        jsonPayload = {
                    "clientTimestamp": int(time.time() * 1000),
                    "solution":str(g.graph),
                    }

        p = send( 'POST', '/vault/1.0.0', json.dumps( jsonPayload ) )
        g.is_to_validation = True
        g.coin_status = p[ 'status' ]
        g.coin_id = p[ 'coinId' ]
        g.save()

def master_sync():
    h = "api_key="+helper_util.get_master_api_key[ "dev" ]
    for m in Master.objects.all():
        continue
        path = "/cloud/master/masters?%s"%h
        conn = httplib.HTTPConnection( m.url, port=m.port )
        conn.request( 'GET', path )
        b = BytesIO( conn.getresponse().read() )
        try:
            resp = JSONParser().parse( b )
        except ParseError:
            print "parse error"
            continue
        for r in resp:
            if Master.objects.filter(url=r["url"]).count() < 1:
                master = Master( url=r["url"],
                                 port=r["port"],
                                 last_seen=r["last_seen"] )
                master.save()
        conn.close()

    first_sync_list = Graph.objects.all()
    sync_list = Graph.objects.filter( is_synced = False )
    for m in Master.objects.all():
        if m.first_sync:
            serializer = MasterGraphSerializer( first_sync_list, many=True )
        else:
            serializer = MasterGraphSerializer( sync_list, many=True )
        print serializer
        conn = httplib.HTTPConnection( m.url, port=m.port )
        conn.request( 'POST', '/cloud/master/graph_sync' )
        conn.close()
        

def send( method, path, payload=None ):
    bank_key = helper_util.get_bank_key( 'dev' )
    if bank_key is None:
        return

    conn = httplib.HTTPSConnection( helper_util.get_bank_connection_url() )
    headers = { "Authorization":"Bearer %s"%bank_key }
    if method=='POST':
        headers['Content-type']='application/json'
        conn.request( method, path, payload, headers=headers )
    else:
        conn.request( method, path, headers=headers )

    b = BytesIO( conn.getresponse().read() )
    p = JSONParser().parse( b )
    conn.close()
    return p

def main():
        if sys.argv[ 1 ] == 'submit':
            check_coins_to_validation()
            send_to_validation()
        elif sys.argv[ 1 ] == 'sync':
            master_sync()
        elif sys.argv[ 1 ] == 'all':
            check_coins_to_validation()
            send_to_validation()
            master_sync()
        else:
            print "error"
        return

main()
