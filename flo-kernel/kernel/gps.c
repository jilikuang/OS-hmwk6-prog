/**
 * Implementation of GPS system call
 */
#include <linux/kernel.h>
#include <linux/syscalls.h>
#include <linux/gps.h>
#include <linux/rwlock.h>
#include <linux/uaccess.h>
#include <linux/time.h>
#include <linux/namei.h>
#include <linux/limits.h>

#include <linux/slab.h>

/* debug define */
#define log	printk
#define	R_OK	4 

/* my own repository */
static struct gps_kdata s_kdata = {
	.m_lat = {0, 0, 0, 0, 0, 0, 0, 0},
	.m_lon = {0, 0, 0, 0, 0, 0, 0, 0},
	.m_acc = {0, 0, 0, 0},
	.m_age = {0, 0, 0, 0},
};

static DEFINE_RWLOCK(s_lock);

SYSCALL_DEFINE1(set_gps_location, struct gps_location __user *, loc)
{
	long retval = 0;
	unsigned long sz = sizeof(struct gps_location);
	struct gps_location kloc;

	/* safety check */
	if (loc == NULL)
		log("loc null failure\n");
		return -EINVAL;

	if (copy_from_user(&kloc, loc, sz) != 0) {
		log("copy_from_user failure\n");
		return -EINVAL;
	} 

	write_lock(&s_lock);

	memcpy (s_kdata.m_lat, &loc->latitude, sizeof(double));
	memcpy (s_kdata.m_lon, &loc->longitude, sizeof(double));
	memcpy (s_kdata.m_acc, &loc->accuracy, sizeof(float));
	memcpy (s_kdata.m_age, &CURRENT_TIME_SEC, sizeof(unsigned long));

	write_unlock(&s_lock);
	return retval;
}

SYSCALL_DEFINE2(get_gps_location, const char __user *, pathname, struct gps_location __user *, loc){
	long retval = 0;
	char * s_kpathname;
	struct gps_location s_kloc;
	struct inode *file_ind;
	struct path s_kpath;

	s_kpathname = kmalloc(sizeof(char)*PATH_MAX, GFP_KERNEL);

	if (s_kpathname == NULL)
		return -ENOMEM;

	if (strncpy_from_user(s_kpathname, pathname, PATH_MAX) < 0) { 
		kfree(s_kpathname);
		return -EFAULT;
	}

	if (sys_access(s_kpathname, R_OK) <0 ) { 
		kfree(s_kpathname);
		return -EACCES;
	}

	if (kern_path(s_kpathname, LOOKUP_FOLLOW | LOOKUP_AUTOMOUNT, &s_kpath) != 0)
		return -EAGAIN;

	file_ind = s_kpath.dentry->d_inode;

	if (file_ind == NULL)
		return -EINVAL;

	/* check if the file is in ext3 */
	if (!(file_ind->i_op->get_gps_location))
		return -ENODEV;

	/* get the gps loc of a file */
	retval = file_ind->i_op->get_gps_location(file_ind, &s_kloc);

	if (copy_to_user(loc, &s_kloc, sizeof(struct gps_location)) != 0) {
		kfree(s_kpathname);
		return -EFAULT;
	}

	return retval;
}

void get_gps_data(struct gps_kdata *data)
{
	read_lock(&s_lock);
	
	memcpy (data->m_lat, s_kdata.m_lat, sizeof(double));
	memcpy (data->m_lon, s_kdata.m_lon, sizeof(double));
	memcpy (data->m_acc, s_kdata.m_acc, sizeof(float));
	memcpy (data->m_age, s_kdata.m_age, sizeof(unsigned long));

	read_unlock(&s_lock);
}

