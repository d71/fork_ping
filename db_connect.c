// multi ping fddkiller@mail.ru
// 2014

#include <mysql/mysql.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <time.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <stdlib.h>

#include <arpa/inet.h>
#include <strings.h>
#include <string.h>

#define BUF_SIZE 256
#define MAX_NODES 10

char server[BUF_SIZE] = "localhost";
char user[BUF_SIZE] = "ping";
char password[BUF_SIZE] = "mypingtest";
char database[BUF_SIZE] = "ping";
char table_hosts[BUF_SIZE] = "hosts";
char table_timeline[BUF_SIZE] = "timeline";
char net_port[BUF_SIZE] = "5008";

main(int argc, char * argv[]){

    time_t time_start,time_end; //для измерения времени исполнения
    time_start = time(NULL);


    // connect to DB
    MYSQL *conn;
    MYSQL_RES *res;
    MYSQL_ROW row;
    
    read_config(argv[1]);
   
    int live=0;
    char query_string[256];
    memset(query_string,0,sizeof(query_string));
   
    conn = mysql_init(NULL);
	/* Connect to database */
    if (!mysql_real_connect(conn, server,
        user, password, database, 0, NULL, 0)) {
        fprintf(stderr, "%s\n", mysql_error(conn));
        exit(1);
    }
    // end connect to DB

    // get ip hosts for ping
    char hosts[1024][20];
    int num_host=0;

    sprintf(query_string,"select ip from %s order by ip",table_hosts);

    mysql_query(conn,query_string);
    res = mysql_store_result(conn);

    while( row=mysql_fetch_row(res) ){
	strcpy(hosts[num_host],row[0]);
	//printf("%d %s\n",num_host,hosts[num_host]);
	num_host++;
    }
    num_host--; //сука пролетает

    mysql_free_result(res);
    ///
    

    //setup tcp server
    //
    int server_sockfd, client_sockfd;
    int server_len, client_len;
    struct sockaddr_in server_address;
    struct sockaddr_in client_address;
    int result;
    fd_set readfds, testfds;
    int port;
 
    port = atoi(net_port);
 
    char buf[BUF_SIZE];

	server_sockfd = socket(AF_INET, SOCK_STREAM, 0);
	server_address.sin_family = AF_INET;
	server_address.sin_addr.s_addr = INADDR_ANY;
	server_address.sin_port = htons(port);
	server_len = sizeof(server_address);
	bind(server_sockfd, (struct sockaddr *)&server_address, server_len);

	listen(server_sockfd, num_host); //кол декрипторов соединений
	FD_ZERO(&readfds);
	FD_SET(server_sockfd, &readfds);
    // end of server setup




    //делимся на процессы
    int n;//кол форков
    for(n=num_host;n>=0;n--){
	pid_t pid=fork();
	if(!pid){
	    int live;
	    live = c_ping(hosts[n]);
	    printf("my host(n)=%d pid=%d host=%s live=%d\n",n,getpid(),hosts[n],live);
	    
	    /// send result to server 127.0.0.1:5008
	        int forkfd;
		int fork_len;
		struct sockaddr_in fork_address;
		int fork_result = -1;
		char fork_buf[BUF_SIZE]; memset(fork_buf,0,sizeof(fork_buf));
 
		forkfd = socket(AF_INET, SOCK_STREAM, 0);

		fork_address.sin_family = AF_INET;
		fork_address.sin_addr.s_addr = inet_addr("127.0.0.1");
		fork_address.sin_port = htons(port);
		fork_len = sizeof(fork_address);
		
		int try=10;
		
		while (fork_result == -1){
		    fork_result = connect( forkfd, (struct sockaddr *)&fork_address, fork_len );
			if ( fork_result == -1 ) {
			    try--;
			    perror("oops : client error");
			    sleep(2);
			    //exit(1);
			}else{
			    sprintf( fork_buf,"%s %d",hosts[n],live );
			    write( forkfd, fork_buf, strlen(fork_buf) );
			    close( forkfd );
			}
			
			if(try==0){
			    //close( forkfd );
	    		    exit(0);
			}
		}
	    ///	
	    exit(0); // fork exit!!!
	}
    }
    
    // end of forks



    int fork_nodes; fork_nodes=num_host+1; //сколько ждать результатов в штуках

    // white network connections
    while(1) {

	char ch;
	int fd;
	int nread;
	testfds = readfds;
	//printf("server waiting port=%d\n",port);
	result = select(FD_SETSIZE, &testfds, (fd_set *)0,(fd_set *)0, (struct timeval *)0);
	if (result < 1) {
	    perror("node");
	    exit(1);
	}
	for (fd = 0; fd < FD_SETSIZE; fd++) {
	    if (FD_ISSET(fd, &testfds)) {

		if (fd == server_sockfd) {
    		    client_len = sizeof(client_address);
    		    client_sockfd = accept(server_sockfd,(struct sockaddr*)&client_address, &client_len);
    		    FD_SET(client_sockfd, &readfds);
    		    printf("connect fork process on fd %d\n", client_sockfd);
		}else {
    		    ioctl(fd, FIONREAD, &nread);
    		    if (nread == 0) {
    			close(fd);
    			FD_CLR(fd, &readfds);
    			printf("disconnect fork process on fd %d\n", fd);
			fork_nodes--;

    		    } else {

			/////////////////////////
			// РАБОТАЕМ С ПРИШЕДШИМ
     
			memset(buf,0,sizeof(buf));
    			recv(fd, buf, BUF_SIZE-1,0);

			// разделяем строку пришедшую на команды и параметры, разделитель - пробел
			char cmd_line[10][50];
			int cmd_i=0;
			char *cmd_tmp, *last;
	    
			cmd_tmp=strtok_r(buf, " ",&last);

			while(cmd_tmp!=NULL){
			    strcpy(cmd_line[cmd_i],cmd_tmp);
			    cmd_i++;
			    cmd_tmp = strtok_r(NULL," ",&last);
			}
		    
			//// exec SQL query 
			sprintf(query_string,"insert into %s (host,time1,ping) values ('%s',now(),'%s');",table_timeline,cmd_line[0],cmd_line[1]);
			mysql_query(conn,query_string);

			//res = mysql_use_result(conn);
			
			//// exec SQL query
			sprintf(query_string,"update %s set life='%s' where ip='%s' ;",table_hosts,cmd_line[1],cmd_line[0]);
			mysql_query(conn,query_string);

			//if ( mysql_query(conn, query_string) ) {
			//    fprintf(stderr, "%s\n", mysql_error(conn));
			//    exit(1);
			//}
			
			//res = mysql_use_result(conn);
			
			printf("wite %d nodes\n",fork_nodes);

			
			////
		    
		    }
		}
	    }
	}

	if (fork_nodes==0) break;
    }

    /* close connection */
    //mysql_free_result(res);
    mysql_close(conn);

    printf ("DB connection closed\n");
    
    time_end = time(NULL);
    
    printf ( "work time %f sec\n",difftime(time_end, time_start) );
    
    return;
}


int read_config (char *config_file) { 

    FILE *file;
    
    int N=256;
    
    char arr[N];
 
    file = fopen(config_file, "r");
 
    while (fgets (arr, N, file) != NULL){

	arr[strlen(arr)-1]='\0';

	if(strstr(arr,"=")!=0){
	    
	    
	    // разделяем строку пришедшую на команды и параметры, разделитель - пробел
	    char cmd_line[2][50];
	    int cmd_i=0;
	    char *cmd_tmp, *last;
	    
	    cmd_tmp=strtok_r(arr, "=",&last);

	    while(cmd_tmp!=NULL){
		strcpy(cmd_line[cmd_i],cmd_tmp);
		cmd_i++;
		cmd_tmp = strtok_r(NULL,"=",&last);
	    }

	    if( strstr(cmd_line[0],"db_host") )		strcpy(server,cmd_line[1]);
	    if( strstr(cmd_line[0],"db_user") )		strcpy(user,cmd_line[1]);
	    if( strstr(cmd_line[0],"db_password") )	strcpy(password,cmd_line[1]);
	    if( strstr(cmd_line[0],"db_name") )		strcpy(database,cmd_line[1]);
	    if( strstr(cmd_line[0],"table_hosts") )	strcpy(table_hosts,cmd_line[1]);
	    if( strstr(cmd_line[0],"table_timeline") )	strcpy(table_timeline,cmd_line[1]);
	    if( strstr(cmd_line[0],"net_port") )	strcpy(net_port,cmd_line[1]);
	
	}
    }
    
    fclose(file);

    return;
}