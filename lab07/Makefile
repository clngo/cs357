all: forkit tryit

forkit : forkit.c
	gcc -o forkit -Wall forkit.c

tryit: tryit.c
	gcc -o tryit -Wall tryit.c

clean:
	rm forkit
	rm tryit