/**
 * Implementation of GPS system call
 */

#include <linux/syscalls.h>
#include <linux/gps.h>
#include <linux/rwlock.h>
#include <linux/uaccess.h>

/* debug define */
#define log	printk

/* my own repository */
static struct gps_kdata s_kdata;
static DEFINE_RWLOCK(s_lock);

SYSCALL_DEFINE1(set_gps_location, struct gps_location __user *, loc)
{
	long retval = 0;
	unsigned long sz = sizeof(struct gps_location);
	struct gps_location kloc;

	/* safety check */
	if (loc == NULL)
		return -EINVAL;

	if (copy_from_user(&kloc, loc, sz) != sz) {
		log("copy_from_user failure\n");
		return -EINVAL;
	} 

	write_lock(&s_lock);

	memcpy (s_kdata.m_lat, &loc->latitude, sizeof(double));
	memcpy (s_kdata.m_lon, &loc->longitude, sizeof(double));
	memcpy (s_kdata.m_acc, &loc->accuracy, sizeof(float));

#if 0
	s_kdata.m_age = /* get current time */
#endif

	write_unlock(&s_lock);
	return retval;
}

void get_gps_data(struct gps_kdata *data)
{
	read_lock(&s_lock);
	
	memcpy (data->m_lat, s_kdata.m_lat, sizeof(double));
	memcpy (data->m_lon, s_kdata.m_lon, sizeof(double));
	memcpy (data->m_age, s_kdata.m_acc, sizeof(float));

#if 0
	s_kdata.m_age = /* get current time */
#endif
	read_unlock(&s_lock);
}

