# multi thread ping by d71 20141204

all: db_connect.o c_ping.o
	gcc -o fork_ping db_connect.o `mysql_config --cflags --libs` c_ping.o
    
c_ping.o: c_ping.c
	gcc -c c_ping.c
    
db_connect.o: db_connect.c
	gcc -c db_connect.c 
    
clean:
	rm -f *.o fork_ping