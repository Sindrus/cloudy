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
    if not helper_util.authorized_api_key( request.GET.get( 'api_key', '' ) ):
        return HttpResponse( status=404 )
    timelimit = timezone.now() + timedelta( hours=-5 )
    return_list = Master.objects.filter( last_seen__lt=timelimit )
    serialized = MasterSerializer( return_list, many=True )
    return JSONResponse( serialized.data )
