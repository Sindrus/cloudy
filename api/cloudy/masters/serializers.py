from rest_framework import serializers

from masters.models import Master

class MasterSerializer( serializers.ModelSerializer ):
    class Meta:
        model = Master
        fields = ('url','port', 'last_seen')
