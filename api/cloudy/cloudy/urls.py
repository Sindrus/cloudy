from django.conf.urls import patterns, include, url
from django.http import HttpResponse

urlpatterns = patterns('',
    # Examples:
    url( r'^$', lambda response: HttpResponse( "Hello l" ) ),
    url( r'^cloud/slave/graphs$', 'graph.views.graph_list' ),
    url( r'^cloud/slave/graphs/(?P<pk>[0-1]+)$', 'graph.views.graph_detail' ),
    url( r'^cloud/slave/new$', 'graph.views.new' ),
    url( r'^cloud/slave/save$', 'graph.views.save' ),
    url( r'^cloud/slave/solution$', 'graph.views.solution' ),
    url( r'^cloud/master/masters$', 'masters.views.get_masters' ),
)
