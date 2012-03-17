#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/sched.h>	/* For current */
#include <linux/tty.h>		/* For the tty declarations */
#include <linux/version.h>

void conivent_printf(char *str, ...) {
	struct tty_struct *my_tty;
    
    va_list args;
    int str_length;
    char temp_str[200];
    
    va_start(args, str);
    str_length = vsprintf(temp_str, str, args);
    printk("str_length %d\n", str_length);
    va_end(args);
    temp_str[str_length] = '\0';
    
    
#if ( LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,5) )
	my_tty = current->tty;
#else
	my_tty = current->signal->tty;
#endif
    
	if (my_tty != NULL) {
        
		((my_tty->driver)->ops->write) (my_tty,	/* The tty itself */
                                        temp_str,	/* String                 */
                                        strlen(temp_str));	/* Length */
        
		((my_tty->driver)->ops->write) (my_tty,  "\015\012", 2);
	}
}


void print_string(char *str) {
	struct tty_struct *my_tty;
    
	/* 
	 * tty struct went into signal struct in 2.6.6 
	 */
#if ( LINUX_VERSION_CODE <= KERNEL_VERSION(2,6,5) )
	/* 
	 * The tty for the current task 
	 */
	my_tty = current->tty;
#else
	/* 
	 * The tty for the current task, for 2.6.6+ kernels 
	 */
	my_tty = current->signal->tty;
#endif
    
	/* 
	 * If my_tty is NULL, the current task has no tty you can print to 
	 * (ie, if it's a daemon).  If so, there's nothing we can do.
	 */
	if (my_tty != NULL) {
        
		/* 
		 * my_tty->driver is a struct which holds the tty's functions,
		 * one of which (write) is used to write strings to the tty. 
		 * It can be used to take a string either from the user's or 
		 * kernel's memory segment.
		 *
		 * The function's 1st parameter is the tty to write to,
		 * because the same function would normally be used for all 
		 * tty's of a certain type.  The 2nd parameter controls whether
		 * the function receives a string from kernel memory (false, 0)
		 * or from user memory (true, non zero).  The 3rd parameter is
		 * a pointer to a string.  The 4th parameter is the length of
		 * the string.
		 */
		((my_tty->driver)->ops->write) (my_tty,	/* The tty itself */
                                        str,	/* String                 */
                                        strlen(str));	/* Length */
        
		/* 
		 * ttys were originally hardware devices, which (usually) 
		 * strictly followed the ASCII standard.  In ASCII, to move to
		 * a new line you need two characters, a carriage return and a
		 * line feed.  On Unix, the ASCII line feed is used for both 
		 * purposes - so we can't just use \n, because it wouldn't have
		 * a carriage return and the next line will start at the
		 * column right after the line feed.
		 *
		 * This is why text files are different between Unix and 
		 * MS Windows.  In CP/M and derivatives, like MS-DOS and 
		 * MS Windows, the ASCII standard was strictly adhered to,
		 * and therefore a newline requirs both a LF and a CR.
		 */
		((my_tty->driver)->ops->write) (my_tty,  "\015\012", 2);
	}
}
