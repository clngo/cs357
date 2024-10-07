#include <stdio.h>
#define TABCOUNT 8
#define SPACE ' '
#define TAB '\t'
#define BACKSPACE '\b'
#define ENTER '\n'
#define RETURN '\r'

int main(int argc, char *argv[]) {
	int c, i, count;
	count = 0; /* Start the count at 0 */

	/* Finish program when you are at the end of the file */
	while ((c=getchar()) != EOF) {
		if (c == TAB) {
			/* Print spaces */
			for (i = count % TABCOUNT; i < TABCOUNT; i++)  {
				putchar(SPACE);
				count++;
			}	
		}
		else {	
			putchar(c);

			/* Decrement count unless at the start of the line */
			if (c == BACKSPACE && count > 0) {	
				count--;
			}			
			else {
				count++;

				/* Reset the count at the start of a new line */
				if (c == ENTER || c == RETURN) {
					count = 0;
				}
			}
							
		}
	}
	return 0;
}
	
 

