
ROOT=$(shell pwd)
#查找mysql头文件的路径
#I表示指定路径，-l表示指定库
#动态库，，在指定路径下进行查找
INCLUDE=-I$(ROOT)/../../lib/include
LIB=-L$(ROOT)/../../lib/lib
LIB_NAME=-lmysqlclient
#LD_FLAG=-static #表示静态连接

.PHONY:all
all:netCal insertdata selectdata
netCal:netCal.c
	gcc -g -o $@ $^
insertdata:insertdata.c
	gcc -g -o $@ $^ $(INCLUDE)  $(LIB) $(LIB_NAME)
selectdata:selectdata.c
	gcc -g -o $@ $^ $(INCLUDE)  $(LIB) $(LIB_NAME)
.PHONY:clear
clear:
	rm -rf netCal insertdata selectdata
