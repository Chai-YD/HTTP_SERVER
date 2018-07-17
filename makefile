ROOT=$(shell pwd)
.PHONY:all
all:httpd cgi



httpd:httpd.c
	gcc -g -o $@ $^ -pthread
.PHONY:clear

cgi:
	cd $(ROOT)/wwwroot/cgi; make clear; make; cd -

.PHONY:output
output:
	mkdir -p output/wwwroot/cgi
	mkdir -p output/wwwroot/lib
	cp httpd output
	cp start.sh output
	cp -rf lib/lib output/lib
	cp -f wwwroot/*.html output/wwwroot
	cp -rf wwwroot/imag output/wwwroot
	cp -f wwwroot/cgi/netCal output/wwwroot/cgi
	cp -f wwwroot/cgi/insertdata output/wwwroot/cgi
	cp -f wwwroot/cgi/selectdata output/wwwroot/cgi
clear:
	rm -f httpd
	cd $(ROOT)/wwwroot/cgi;make clear;cd -
	rm -rf output
