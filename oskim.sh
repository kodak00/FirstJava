#!/bin/sh

export QUERY_STRING="&tm=201902200635&stn=KWK&rdr=RAW&vol=RN&cpi=PPI&cdf=1&sms=0&swpn=0&ht=1.5&map=&color=&area=2&ang=&size=880&zoom_level=0&zoom_x=0000000&zoom_y=0000000&ZRa=200&ZRb=1.6&aws=1&gov=KMA&rand=211733"


#/srv/kres/rdr/www/cgi-bin/rdr/nph-rdr_stn1_img > oskim.png
cgdb /srv/kres/rdr/www/cgi-bin/rdr/nph-rdr_stn1_img
