#include<stdio.h>
#include<mysql.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<string.h>

//mysql的底层是TCP协议，网络端口号为3306
//netstat -nltp  进行查看
//host，user,passed,db,port,unix_socket,client_
void select_data(){
    MYSQL *mysql_fd = mysql_init(NULL);
    if(mysql_real_connect(mysql_fd,"127.0.0.1","root","","bit",3306,NULL,0) == NULL){
        printf("connect failed\n");
        return;
    }
    printf("connect mysql success!\n");
    char sql[1024];
    sprintf(sql,"select * from bit2");
    mysql_query(mysql_fd,sql);
    MYSQL_RES *res = mysql_store_result(mysql_fd);//
    int row = mysql_num_rows(res);
    int col = mysql_num_fields(res);
    MYSQL_FIELD *field = mysql_fetch_field(res);//列明
    int i = 0;
    for(;i<col;++i){
        printf("%s\t",field[i].name);
    }
    printf("\n");
    //拿数据
    printf("<table border=\"1\">");
    for(i = 0;i<row;++i){
        MYSQL_ROW rowdata = mysql_fetch_row(res);
        int j = 0;
        printf("<tr>");
        for(;j<col;++j){
            printf("<td>%s</td>",rowdata[j]);
        }
        printf("</tr>");  
    }
    printf("</table>"); 
    mysql_close(mysql_fd);
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
    select_data();
    //printf("client version: %s\n",mysql_get_client_info());
    //printf("hello\n");
    return 0;
}
