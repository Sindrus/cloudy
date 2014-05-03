from datetime import timedelta

from django.views.decorators.csrf import csrf_exempt
from django.utils import timezone
from rest_framework.renderers import JSONRenderer
from django.http import HttpResponse
from masters.models import Master
from masters.serializers import MasterSerializer

import helper_util

class JSONResponse( HttpResponse ):
    def __init__( self, data, **kwargs ):
        content = JSONRenderer().render( data )
        kwargs['content_type'] = 'application/json'
        super(JSONResponse, self).__init__(content, **kwargs)

# Create your views here.
@csrf_exempt
def get_masters( request ):
    print request.GET
    print "api_key",request.GET.get( 'api_key', '' )
    if not helper_util.authorized_api_key( request.GET.get( 'api_key', '' ) ):
        return HttpResponse( status=404 )
    timelimit = timezone.now() + timedelta( hours=-5 )
    return_list = Master.objects.filter( last_seen__gt=timelimit )
    serialized = MasterSerializer( return_list, many=True )
    print "returning"
    return JSONResponse( serialized.data )

@csrf_exempt
def graph_sync( request ):
    b = BytesIO( request.body )
    p = JSONParser().parse( b )

    is_list = isinstance( p, list )
    try:
        if is_list:
            for l in p:
                g = Graph.objects.get( graph_id=l[ 'graph_id' ] )
                if g.last_updated > l[ 'last_updated' ]:
                    continue
                g.is_synced = False
                g.save()
                for t in Taboo.objects.filter( graph=g ):
                    t.delete() 
        else:
            g = Graph.objects.get( graph_id=p[ 'graph_id' ] )
            if g.last_uptdated < l[ 'last_updated' ]:
                for t in Taboo.objects.filter( graph=g ):
                    t.delete() 
            g.is_synced = False
            g.save()
    except:
        print "An error has occurded in masters.views.graph_sync"
        pass
    serializer = MasterGraphSerializer( data=p, many=is_list )
    if serializer.is_valid():
        serializer.save()
    else:
        print serializer.errors
    return HttpResponse( "saved\n" )

