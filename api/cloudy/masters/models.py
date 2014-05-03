from django.db import models

# Create your models here.
class Master( models.Model ):
    url = models.CharField( max_length=255 )
    port = models.IntegerField( default=80 )
    last_seen = models.DateTimeField()
    first_sync = models.BooleanField( default=False )
