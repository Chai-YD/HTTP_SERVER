#include<stdio.h>
#include<mysql.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<string.h>

//mysql的底层是TCP协议，网络端口号为3306
//netstat -nltp  进行查看
//host，user,passed,db,port,unix_socket,client_
void insert_data(char* name){
//void insert_data(){
    MYSQL *mysql_fd = mysql_init(NULL);
    if(mysql_real_connect(mysql_fd,"127.0.0.1","root","","bit",3306,NULL,0) == NULL){
        printf("connect failed\n");
        return;
    }
    printf("connect mysql success!\n");
    char sql[1024];
    //printf("arg:%s\n",name);
    sprintf(sql,"insert into bit2 (name) values(\"%s\")",name);
    printf("SQL:%s\n",sql);
    mysql_query(mysql_fd,sql);
    //mysql_query(mysql_fd,"insert into bit2 (name) \
            values(\"tianliu\")");
    mysql_close(mysql_fd);
    printf("chenggong\n");
}

int main(){
    char data[1024];
    if(getenv("METHOD")){
        if(strcasecmp(getenv("METHOD"),"GET") == 0){
            strcpy(data,getenv("QUERY_STRING"));
        }else{
            int content_Length = atoi(getenv("CONTENT_LENTH"));
            int i =0 ;
            for(;i<content_Length;++i){
                read(0,data+i,1);
            }
            data[i] = 0;
        }
    }
    printf("arg:%s\n",data);
    char* name;
    //此处sscanf不能分清%s的边界
    //sscanf(data,"name=%s&id=%d",name,&id);
    strtok(data,"=&");
    name = strtok(NULL,"=&");
    insert_data(name);
    //insert_data();
    //printf("client version: %s\n",mysql_get_client_info());
    //printf("hello\n");
    return 0;
}
