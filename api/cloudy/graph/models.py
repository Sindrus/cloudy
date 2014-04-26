from django.db import models

# Create your models here.
class Graph (models.Model):
    graph = models.TextField()
    last_updated = models.DateTimeField()
    in_progress = models.BooleanField()
    matrix_size = models.IntegerField()
    graph_id = models.BigIntegerField()
    coin_id = models.CharField( max_length=255, blank=True, null=True )
    coin_status = models.IntegerField( blank=True, null=True )
    error_code = models.IntegerField( blank=True, null=True )
    is_solution = models.BooleanField( default=False )
    passed_isomorph_check = models.BooleanField( default=False )
    is_to_validation = models.BooleanField( default=False )
    is_synced = models.BooleanField( default=False )

class Taboo( models.Model ):
    graph = models.ForeignKey( Graph, related_name='taboos' )
    age = models.IntegerField() # Way to sort the taboolist
    move = models.TextField()
