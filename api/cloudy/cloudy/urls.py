from django.conf.urls import patterns, include, url
from django.http import HttpResponse

from django.contrib import admin
admin.autodiscover()

urlpatterns = patterns('',
    # Examples:
    # url(r'^$', 'cloudy.views.home', name='home'),
    # url(r'^blog/', include('blog.urls')),
    url( r'^$', lambda response: HttpResponse( "Hello" ) ),
    # url(r'^admin/', include(admin.site.urls)),
)
