import math
from datetime import datetime

from pytz import timezone as ptz

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

class MasterGraphSerializer( serializers.ModelSerializer ):
    taboos = TabooSerializer( many=True, required=False )

    class Meta:
        model = Graph

    def restore_object( self, attrs, instance=None ):
        graph_id = attrs.get( 'graph_id' ) if attrs.get( 'graph_id' )\
                else helper_util.gen_id()
        if Graph.objects.filter( graph_id = graph_id ).count() < 1:
            instance = Graph.objects.get( graph_id = graph_id )
            if instance.last_updated > attrs.get( 'last_updated' ):
                return instance
        else:
            instance = Graph( graph_id = graph_id )

        instance.in_progress = attrs.get( 'in_progress' )
        instance.matrix_size = attrs.get( 'matrix_size' )
        instance.graph = attrs.get( 'graph' )
        instance.last_updated = attrs.get( 'last_updated' )
        instance.coin_id = attrs.get( 'coin_id' )
        instance.coin_status = attrs.get( 'coin_status' )
        instance.error_code = attrs.get( 'error_code' )
        instance.is_solution = attrs.get( 'is_solution' )
        instance.passed_isomorph_check = attrs.get( 'passed_isomorph_check' )
        instance.is_to_validation = attrs.get( 'is_to_validation' )
        instance.is_synced = attrs.get( 'is_synced' )

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
        
        d = attrs.get( 'last_updated' )
        d = d.replace( tzinfo=ptz( 'UTC' ) )
        try:
            instance = Graph.objects.get( graph_id = graph_id )
            if instance.last_updated > d:
                return instance
        except Graph.DoesNotExist:
            instance = Graph( graph_id = graph_id )

        instance.in_progress = attrs.get( 'in_progress' )
        instance.matrix_size = attrs.get( 'matrix_size' )
        instance.graph = d
        instance.last_updated = attrs.get( 'last_updated' )

        return instance
