#include<stdio.h>
#include<stdlib.h>
#include<strings.h>
#include<string.h>
#include<unistd.h>


#define MAX 1024

void mycal(char* buff){
    //获取数据strtok
    //sprintf函数，sscanf函数（格式化输出）
    int x,y;
    //echo Server :GET echo Server arg:firstdata=100&lastdata=20 
    sscanf(buff,"firstdata=%d&lastdata=%d",&x,&y);
    printf("firstdata=%d  lastdata=%d\n",x,y);
    printf("<html>\n");
    printf("<body>\n");
    printf("<h3>%d + %d = %d</h3>\n",x,y,x+y);
    //除法与取摸要进行判断，除零错误
    printf("</body>\n");
    printf("</html>\n");
}

int main(){
    char buff[MAX];
    if(getenv("METHOD")){
        if(strcasecmp(getenv("METHOD"),"GET")){
            strcpy(buff,getenv("QUERY_STRING"));
        }else{
            int content_Length = atoi(getenv("CONTENT_LENGTH"));
            int i = 0;
            char c;
            for(;i<content_Length;++i){
                read(0,&c,1);
                buff[i] = c;
            }
            buff[i] = '\0';
        }
    }
   // 走在这里，数据已经拿到了
   // 进行计算
   // 打印数据
    //echo Server :GET echo Server arg:firstdata=100&lastdata=20 
    printf("echo Server :%s\n",getenv("METHOD"));
    printf("echo Server arg:%s\n",getenv("QUERY_STRING"));
    printf("hello BAT\n");
    mycal(buff);
    return 0;
}
