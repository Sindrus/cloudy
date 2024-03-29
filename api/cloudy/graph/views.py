import math
from datetime import timedelta

from django.http import HttpResponse
from django.views.decorators.csrf import csrf_exempt
from django.utils import timezone
from rest_framework.renderers import JSONRenderer
from rest_framework.parsers import JSONParser
from rest_framework.response import Response
from rest_framework.compat import BytesIO
from rest_framework import status

from graph.models import Graph, Taboo
from graph.serializers import GraphSerializer
import helper_util
# Create your views here.

class JSONResponse( HttpResponse ):
    def __init__( self, data, **kwargs ):
        content = JSONRenderer().render( data )
        kwargs['content_type'] = 'application/json'
        super(JSONResponse, self).__init__(content, **kwargs)

@csrf_exempt
def graph_list( request ):
    if request.method == 'GET':
        graph = Graph.objects.all()
        serializer = GraphSerializer( graph, many = True )
        return JSONResponse( serializer.data )
    elif request.method == 'POST':
        data = JSONParser().parse( request )
        serializer = GraphSerializer( data = data )
        if serializer.is_valid():
            serializer.save()
            return JSONResponse( serializer.data, status=201 )
        return JSONResponse(serializer.errors, status=400)

@csrf_exempt
def graph_detail( request, pk ):
    try:
        graph = Graph.objects.get( graph_id = pk )
    except Graph.DoesNotExist:
        return HttpResponse( status = 404 )
        
    if request.method == 'GET':
        serializer = GraphSerializer( graph, many=True )
        return JSONResponse( serializer.data )

@csrf_exempt
def new( request ):
    if not request.method == 'GET':
        return HttpResponse( status=401 )
    if not helper_util.authorized_api_key( request.GET.get( 'api_key', '' ) ):
        return HttpResponse( status=401 )
    timelimit = timezone.now() + timedelta( minutes=-10 )
    g = Graph.objects.filter( last_updated__lt=timelimit ).order_by( 'last_updated' )
    if( len( g )>0 ):
        g = g[ 0 ]
        g.last_updated = timezone.now()
        g.save()
    else:
        graph_id = helper_util.gen_id()
        graph = helper_util.gen_new_graph()
        g = Graph( graph=graph, in_progress=True, matrix_size=int(math.sqrt(len(graph))),
                last_updated = timezone.now() )
        g.graph_id = graph_id
        g.save()

    serializer = GraphSerializer( g )
    return JSONResponse( serializer.data )

from datetime import datetime
from pytz import timezone as ptz
@csrf_exempt
def save( request ):
    b = BytesIO(request.body)
    p = JSONParser().parse(b)
    is_list = isinstance( p, list )
    if is_list:
        for l in p:
            d = datetime.strptime( l[ 'last_updated' ], "%Y-%m-%d %H:%M:%S")
            d = d.replace( tzinfo=ptz( 'UTC' ) )
            try:
                g = Graph.objects.get( graph_id=l[ 'graph_id' ] )
                if g.last_updated > d:
                    continue
                g.is_synced = False
                g.save()
                for t in Taboo.objects.filter( graph=g ):
                    t.delete() 
            except Graph.DoesNotExist:
                g = Graph( graph_id = l[ 'graph_id' ] )
    else:
        try:
            g = Graph.objects.get( graph_id=p[ 'graph_id' ] )
            d = datetime.strptime( p[ 'last_updated' ], "%Y-%m-%d %H:%M:%S")
            d = d.replace( tzinfo=ptz( 'UTC' ) )
            if g.last_updated < d:
                for t in Taboo.objects.filter( graph=g ):
                    t.delete() 
            g.is_synced = False
            g.save()
        except Graph.DoesNotExist:
            g = Graph( graph_id = p[ 'graph_id' ] )
    serializer = GraphSerializer( data=p, many=is_list )
    if serializer.is_valid():
        serializer.save()
    else:
        print serializer.errors
    return HttpResponse( "lol\n" )

