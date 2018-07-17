//头文件
#include<stdio.h>
#include<string.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<stdlib.h>
#include<pthread.h>
#include<unistd.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<ctype.h>
#include<strings.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<sys/sendfile.h>
#include<sys/wait.h>
#include<signal.h>

#define MAX 1024
#define HOME_PATH "index.html"
//#define DEBUG
int cgi = 0;
//函数实现
void usage(const char* proc){
    printf("[%s][port]\n",proc);
    return;
}

int startup(int port){
    int sock = socket(AF_INET,SOCK_STREAM,0);
    if(sock<0){
        perror("socket");
        exit(2);
    }
    //日志库，，此处不进行编写
    //防止服务器挂掉，重新启动时的时间等待
    int opt=1;
    setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));
    struct sockaddr_in local;
    local.sin_family = AF_INET;
    local.sin_addr.s_addr = htonl(INADDR_ANY);
    local.sin_port = htons(port);
    if(bind(sock,(struct sockaddr *)&local,sizeof(local))<0){
        perror("bind");
        exit(3);
    }
    if(listen(sock,5)<0){
        perror("listen");
        exit(4);
    }
    return sock;
}

//get_line函数的实现（按行读取函数的实现）
int get_line(int sock,char line[],int size){
    //行分割符（\n,\r,\r\n）都应该考虑在内,均视为\n处理
    int c = 'a';
    ssize_t s = 0;
    int i = 0;
    while(i<size-1&&c!='\n'){
        s = recv(sock,&c,1,0);
        if(s > 0){
            if(c=='\r'){
                if(recv(sock,&c,1,MSG_PEEK)>0){
                    if(c != '\n'){
                        //下一个字符不为\n
                        c = '\n';
                    }else{
                        //下一个字符为\n
                        recv(sock,&c,1,0);
                    }
                }
            }
            line[i++] = c;
        }else{
            return -1;
            break;
        }
    }
    line[i] = '\0';
    return i;
}

void clear_head(int sock){
    char line[MAX];
    do{
        get_line(sock,line,sizeof(line));
    }while(strcmp(line,"\n") != 0);
}
//exe_cgi函数实现
//cgi是一个盒子，例如学校就是cgi机制，而我们是来学校学习的，
//我们就是cgi执行程序
int exe_cgi(int sock,char path[],char method[],char* query_string){
    char line[MAX];
    int content_Length = -1;
    char method_env[MAX/32];
    char query_string_env[MAX];
    char content_Length_env[MAX/16];
    if(strcasecmp(method,"GET") == 0){
        //处理数据clear_head
        clear_head(sock);
    }else{//POST
        do{
            get_line(sock,line,sizeof(line));
            if(strncmp(line,"Content-Length: ",16) == 0){
                content_Length = atoi(line+16);
            }
        }while(strcmp(line,"\n")!=0);
        //如果没有Content-Length,直接返回
        if(content_Length == -1){
            return 404;
        }
    }
    //进行sendfile函数的检测，文件已经打开，就不进行错误判断
    //HTTP基于请求响应的，此时必须进行响应（4部分）
    //  /r/n回车换行
    //printf("method:%s ,path:%s\n",method,path);
    sprintf(line,"HTTP/1.0 200 ok\r\n");
    //send 进行发送
    send(sock,line,strlen(line),0);
    sprintf(line,"Content-Type:text/html\r\n");
    send(sock,line,strlen(line),0);
    sprintf(line ,"\r\n");
    send(sock,line,strlen(line),0);
   int input[2];
   int output[2];
   //此时创建管道时，不进行差错处理
   pipe(input);
   pipe(output);
   pid_t id = fork();
   if(id<0){
       return 404;
   }else if(id == 0){
       //child
       //环境变量可以被子进程继承，而部分变量不可以被子进程继承
       //如：虽然子进程可以继承，但是Content-Length还在缓冲区中
       close(input[1]);
       close(output[0]);
       //由于新进程不知道input与output的文件描述符，所以需要进行
       //重定向
       //dup2(int oldfd,int newfd);最后newfd等同与oldfd
       dup2(input[0],0);
       dup2(output[1],1);
       sprintf(method_env,"METHOD=%s",method);
       putenv(method_env);
       if(strcasecmp(method,"GET")==0){
           sprintf(query_string_env,"QUERY_STRING=%s",query_string);
           putenv(query_string_env);
       }else{
           sprintf(content_Length_env,"CONTENT_LENGTH=%d",content_Length);
           putenv(content_Length_env);
       }
       //环境变量具有全局特性
       //环境变量可以被子进程继承
       //环境变量不会被替换替换掉
       execl(path,path,NULL);
       exit(1);
   }else{
       //father
       close(input[0]);
       close(output[1]);
       char c;
       if(strcasecmp(method,"POST") == 0){
           int i = 0;
           for(;i<content_Length;i++){
               read(sock,&c,1);
               write(input[1],&c,1);
           }
       }
       while(read(output[0],&c,1)>0){
           //send函数默认方式为0
           send(sock,&c,1,0);
       }
       waitpid(id,NULL,0);
       close(input[1]);
       close(output[0]);
   }
   return 200;
}
//echo_www函数实现
void echo_www(int sock,char* path,int size,int* err){
    //需要处理报头和空行
    //走在这里，只是简单的返回网页
    //不进行处理请求头部，则数据在在缓冲区中存放
    clear_head(sock);            
    //sendfile  ,,,#define(sendfile.h)
    int fd = open(path,O_RDONLY);
    if(fd<0){
        *err = 404;
        return;
    }
    //进行sendfile函数的检测，文件已经打开，就不进行错误判断
    //HTTP基于请求响应的，此时必须进行响应（4部分）
    char line[MAX];
    //  /r/n回车换行
    sprintf(line,"HTTP/1.0 200 ok\r\n");
    //send 进行发送
    send(sock,line,strlen(line),0);
    sprintf(line,"Content-Type: text/html;image/jpeg\r\n");
    send(sock,line,strlen(line),0);
    sprintf(line ,"\r\n");
    send(sock,line,strlen(line),0);
    //sendfile函数，高效，在内核中直接进行拷贝
    //in_fd 进行读；out_fd 进行写
    sendfile(sock,fd,NULL,size);
    close(fd);
}

void echo_error(int code){
    switch(code){
        case 404:
            break;
        case 501:
            break;
        default:
            break;
    }
}

//hander_request函数的实现
void* hander_request(void* arg){
    //printf("AAAAAAAAAAAAAAAAAAAAAAAAA\n");
    int sock = (int)arg;
    char line[MAX];
    int errcode = 200;
    char method[MAX/32];  //请求方法
    char url[MAX];  //请求资源
    char* query_string = NULL;//请求字符串
    char path[MAX];
#ifdef DEBUG
    do{
        get_line(sock,line,sizeof(line));//按行读取
        printf("%s",line);//将读取行打印出来
    }while(strcmp(line,"\n")!=0);
#else
    if(get_line(sock,line,sizeof(line))<0){
        errcode = 404;
        goto end;
    }
    //get method(获取方法)
    //GET / HTTP/1.1
    int i = 0;
    int  j = 0;
    while(i < sizeof(method)-1 && j < sizeof(line) \
            && !isspace(line[j])){
        method[i] = line[j];
        i++;
        j++;
    }
    method[i] = '\0';
    //这里只进行对GET、POST进行处理
    if(strcasecmp(method,"GET") == 0){
    }else if(strcasecmp(method,"POST") == 0){
        cgi = 1;
    }else{
        errcode = 404;
        goto end;
    }
    //有可能中间有很多空格
    while(j<sizeof(line)&&isspace(line[j])){
        j++;
    }
    //获取请求资源路径
    i = 0;
    while(i<sizeof(url)-1 && j<sizeof(line) && !isspace(line[j])){
        url[i] = line[j];
        i++;
        j++;
    }
    url[i] = '\0';
    //走在这里，说明资源要么是GET，要么是POST
    //例子：如果我们的数据拼接到了url中时一定是GET
    //例子：如果正文无数据，url中也没有数据，则就是GET方法
    //?左边为访问的资源，右边表示输入的参数
    //请求的资源有可能是一个可执行程序
    //此时，必须为GET方法，才进行检测
    if(strcasecmp(method,"GET")==0){
        query_string = url;
        //不为空，为？
        while(*query_string){
            if(*query_string == '?'){
                *query_string = '\0';
                query_string++;
                cgi = 1;
                break;
            }
            //此时表示没找到
            query_string++;
        }
    }
    //method[GET,POST],cgi[0|1],url[],query_string[NULL|arg]
    sprintf(path,"wwwroot%s",url);
    printf("method:%s  url:%s   cgi:%d\n",method,url,cgi);
    if(path[strlen(path)-1] == '/'){
        strcat(path,HOME_PATH);
    }
    printf("method:%s  url:%s path:%s  cgi:%d\n",method,url,path,cgi);
    //走在这里，说明资源已经定位，需要进行判断文件是不是合法
    //stat函数，1.路径，2.stat结构体
    //成功返回0，失败返回-1
    struct stat st;
    if(stat(path,&st)<0){
        errcode = 404;
        goto end;
    }else{
        //如果文件有可执行权限，就以cgi执行
        if(S_ISDIR(st.st_mode)){
            strcat(path,HOME_PATH);
        }else{
            if((st.st_mode & S_IXUSR) || (st.st_mode & S_IXGRP) \
                    ||(st.st_mode & S_IXOTH)){
                cgi = 1;
            }
        }
        if(cgi){
            errcode = exe_cgi(sock,path,method,query_string);
        }else{
            //非cgi执行，直接返回信息，GET，cgj[0]，
            echo_www(sock,path,st.st_size,&errcode);
        }
    }
#endif
end:
    if(errcode !=200){
        echo_error(errcode);
    }   
    close(sock);
}

//主函数
int main(int argc,char* argv[]){
    if(argc !=2){
        usage(argv[0]);
        return 1;
    }
    int listen_sock = startup(atoi(argv[1]));//函数待实现
    signal(SIGPIPE,SIG_IGN);
    //进入循环体
    for(;;){
        struct sockaddr_in client;
        socklen_t len = sizeof(client);
        int new_sock = accept(listen_sock,(struct sockaddr*)&client,&len);
        printf("循环\n");
        if(new_sock<0){
            perror("accept");
            continue;
        }
        printf("success accept!\n");
        pthread_t id;
        pthread_create(&id,NULL,hander_request,(void*)new_sock);//hander_request函数带解决
        //将线程分离，，防止进程进行等待，导致服务器出错
        pthread_detach(id);
    }
}








