from django.db import models

# Create your models here.
class Graph (models.Model):
    graph = models.TextField()
    in_progress = models.BooleanField()
    coin_status = models.IntegerField( blank=True, null=True )
    error_code = models.IntegerField( blank=True, null=True )
    matrix_size = models.IntegerField()
    graph_id = models.BigIntegerField()
    last_updated = models.DateTimeField()

class Taboo( models.Model ):
    graph = models.ForeignKey( Graph, related_name='taboos' )
    age = models.IntegerField() # Way to sort the taboolist
    move = models.TextField()
