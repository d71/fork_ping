all: make_gd_graph.o
	gcc -o make_graph make_gd_graph.o `mysql_config --cflags --libs` -lgd
    
make_gd_graph.o: make_gd_graph.c
	gcc -c make_gd_graph.c 
    
clean:
	rm -f *.o make_graph