/**
 * Implementation of GPS system call
 */

#include <linux/syscalls.h>
#include <linux/gps.h>

SYSCALL_DEFINE1(set_gps_location, struct gps_location __user *, loc)
{
	long retval = 0;

	return retval;
}
