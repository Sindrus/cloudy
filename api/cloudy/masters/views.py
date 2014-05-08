from datetime import timedelta

from django.views.decorators.csrf import csrf_exempt
from django.utils import timezone
from rest_framework.renderers import JSONRenderer
from rest_framework.parsers import JSONParser
from rest_framework.compat import BytesIO
from rest_framework.exceptions import ParseError
from django.http import HttpResponse
from masters.models import Master
from masters.serializers import MasterSerializer

from graph.models import Graph
from graph.serializers import MasterGraphSerializer

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
    try:
        p = JSONParser().parse( b )
    except ParseError:
        return HttpResponse( "Illegal datainput: %s"%b )

    ret_str = ""

#    for g in Graph.objects.all():
#        ret_str = ret_str+str(g)+ ", "
#    ret_str = ret_str+"\n"

    is_list = isinstance( p, list )
    try:
        if is_list:
            for l in p:
              try:
                if Graph.objects.filter( graph_id=l[ 'graph_id' ] ).exists():
                    g = Graph.objects.get( graph_id=l[ 'graph_id' ] )
                    if g.last_updated > l[ 'last_updated' ]:
                        continue
                    g.is_synced = False
                    g.save()
                    for t in Taboo.objects.filter( graph=g ):
                        t.delete() 
              except Graph.DoesNotExist:
                return HttpResponse( "did not find %d"%l['graph_id'] )
        else:
          try:
            if Graph.objects.filter( graph_id=p[ 'graph_id' ] ).exists():
                g = Graph.objects.get( graph_id=p[ 'graph_id' ] )
                if g.last_uptdated < p[ 'last_updated' ]:
                    for t in Taboo.objects.filter( graph=g ):
                        t.delete() 
                g.is_synced = False
                g.save()
          except Graph.DoesNotExist:
            return HttpResponse( "did not find %d"%l['graph_id'])
    except:
        return HttpResponse( "An error has occurded in masters.views.graph_sync"+ret_str )
    serializer = MasterGraphSerializer( data=p, many=is_list )
    if serializer.is_valid():
        serializer.save()
    else:
        print serializer.errors

    for g in Graph.objects.all():
        ret_str = ret_str+str(g.graph_id)+ ", "
    ret_str = ret_str+"\n"
    return HttpResponse( ret_str+"\n" )

