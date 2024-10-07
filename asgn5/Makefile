all : mytalk

mytalk : mytalk.c libtalk.a 
	gcc -g -o mytalk -Wall mytalk.c libtalk.a -l ncurses

clean :
	rm mytalk