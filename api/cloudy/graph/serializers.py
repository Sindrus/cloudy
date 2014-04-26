import math

from rest_framework import serializers
from django.forms import widgets
from django.utils import timezone
from django.core.exceptions import ValidationError
from graph.models import Graph, Taboo

import helper_util

class TabooSerializer( serializers.ModelSerializer ):
    class Meta:
        model = Taboo
        fields = ('graph', 'age', 'move' )

    def restore_object( self, attrs, instance=None ):
        instance = Taboo()
        instance.graph = attrs.get( 'graph' )
        instance.age = attrs.get( 'age' )
        instance.move = attrs.get( 'move' )
        instance.save()
        return instance

class GraphSerializer( serializers.ModelSerializer ):
    taboos = TabooSerializer( many=True, required=False )

    class Meta:
        model = Graph
        fields = (  'graph', 'in_progress', 'matrix_size', 
                    'graph_id', 'last_updated', 'taboos',
                    'is_solution' )

    def restore_object( self, attrs, instance=None ):
        graph_id = attrs.get( 'graph_id' ) if attrs.get( 'graph_id' )\
                else helper_util.gen_id()
        try:
            instance = Graph.objects.get( graph_id = graph_id )
            if instance.last_updated > attrs.get( 'last_updated' ):
                return instance
        except Graph.DoesNotExist:
            instance = Graph( graph_id = graph_id )

        instance.in_progress = attrs.get( 'in_progress' )
        instance.matrix_size = attrs.get( 'matrix_size' )
        instance.graph = attrs.get( 'graph' )
        instance.last_updated = attrs.get( 'last_updated' )

        return instance