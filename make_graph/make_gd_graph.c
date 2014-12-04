#include <mysql/mysql.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <time.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <stdlib.h>

#include <strings.h>
#include <string.h>

#include <gd.h>
#include <gdfontg.h>

#define BUF_SIZE 256

char server[BUF_SIZE];
char user[BUF_SIZE];
char password[BUF_SIZE];
char database[BUF_SIZE];
char table_hosts[BUF_SIZE];
char table_timeline[BUF_SIZE];
char graph_dir[BUF_SIZE];

int main(int argc, char * argv[])
{

    time_t time_start,time_end; //для измерения времени исполнения
    time_start = time(NULL);

    read_config(argv[1]);

    gdImagePtr im1;
    FILE *out;
    int gray, green, dark_green;
    int pic_w = 288;
    int pic_h = 10;



    // connect to DB
    MYSQL *conn;
    MYSQL_RES *res,*res_ip;
    MYSQL_ROW row, row_ip;
//    char *server = "localhost";
//    char *user = "ping";
//    char *password = "mypingtest";
//    char *database = "ping";
   
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

    //printf("DB connected\n");

    // get ip hosts for ping
    char *host;
    char string_ip[256],file_name[256];
    int life;

    sprintf(query_string,"select ip from %s order by ip",table_hosts);

    mysql_query(conn,query_string);
    res = mysql_store_result(conn);

    while( row=mysql_fetch_row(res) ){
	
	host=row[0];

	memset( string_ip,0,sizeof(string_ip) );
    
	sprintf(string_ip,"select ping,time1 from %s WHERE host='%s' order by time1 DESC LIMIT 288",table_timeline,host);
	mysql_query(conn,string_ip);
	res_ip = mysql_store_result(conn);


	im1 = gdImageCreate(pic_w, pic_h);
	gray = gdImageColorAllocate(im1, 220, 220, 220);
	green = gdImageColorAllocate(im1, 0, 200, 0);
	dark_green  = gdImageColorAllocate(im1, 0, 150, 0);

	int i=0;
	while( row_ip=mysql_fetch_row(res_ip) ){
	    
	    life=atoi( row_ip[0] );
	    
	    if( strstr(row_ip[1],"00:00:")==0 ){
		gdImageLine(im1,i,10,i,10-(life*10),green);
	    }else{
		gdImageLine(im1,i,10,i,10-(life*10),dark_green);
	    }
	    
	    i++;
	    
	    
	}

	printf("%s/%s\n",graph_dir,host);
	
	sprintf(file_name,"%s/%s.png",graph_dir,host);
	out = fopen(file_name, "wb");
	gdImagePng(im1, out);
	fclose(out);
	gdImageDestroy(im1);

    }

    mysql_free_result(res);
    mysql_free_result(res_ip);

    mysql_close(conn);

    time_end = time(NULL);
    printf ( "work time %f sec\n",difftime(time_end, time_start) );


    return 0;
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
	    if( strstr(cmd_line[0],"graph_dir") )	strcpy(graph_dir,cmd_line[1]);
	
	}
    }
    
    fclose(file);

    return;
}