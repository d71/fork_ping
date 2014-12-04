#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

#include "c_ping.h"

#define BUF_SIZE 256

int c_ping(char *host) {

    FILE *popen_result;
    char cmd_buf[BUF_SIZE];
    int live=1;
    char cmd[BUF_SIZE] = "ping -n -c3 -w3 ";
    
    strcat( cmd,host );
    
    popen_result = popen(cmd, "r");

    if(!popen_result){
	printf("Can't execute command\n");
    }
    
    int line_count=0;
		
    while(fgets(cmd_buf, sizeof(cmd_buf), popen_result)!=NULL){
	
	if( strstr(cmd_buf,"0 received")!=NULL ){
	    
	    //printf("%d> %s", line_count, cmd_buf);
	    
	    live=0;
	}
	
	memset(cmd_buf, '\0', BUF_SIZE);
	line_count++;
    }
    
    pclose(popen_result);
    
    return live;
    
    //printf("\n host %s live=%d\n\n",argv[1],live);

}