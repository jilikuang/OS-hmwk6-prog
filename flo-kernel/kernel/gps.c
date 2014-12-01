/*
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
	if (loc == NULL){
		log("loc null failure\n");
		return -EINVAL;
	}

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
/*
	log("s_kdata.m_age is: %d%d%d%d%d\n", s_kdata.m_age[0],s_kdata.m_age[1],
		s_kdata.m_age[2],s_kdata.m_age[3]);

	log("s_kdata.m_lat is: %d%d%d%d%d%d%d%d\n", s_kdata.m_lat[0],s_kdata.m_lat[1],
		s_kdata.m_lat[2],s_kdata.m_lat[3],s_kdata.m_lat[4],
		s_kdata.m_lat[5],s_kdata.m_lat[6],s_kdata.m_lat[7]);

	log("s_kdata.m_lon is: %d%d%d%d%d%d%d%d\n", s_kdata.m_lon[0],s_kdata.m_lon[1],
		s_kdata.m_lon[2],s_kdata.m_lon[3],s_kdata.m_lon[4],
		s_kdata.m_lon[5],s_kdata.m_lon[6],s_kdata.m_lon[7]);

	log("s_kdata.m_acc is: %d%d%d%d%d%d%d%d\n", s_kdata.m_acc[0],s_kdata.m_acc[1],
		s_kdata.m_acc[2],s_kdata.m_acc[3]);
*/
	return retval;
}

SYSCALL_DEFINE2(get_gps_location, const char __user *, pathname, struct gps_location __user *, loc){
	int retval = 0;
	char *s_kpathname;
	struct gps_location s_kloc;
	struct inode *file_ind;
	struct path s_kpath;

	if (sys_access(pathname, R_OK) != 0){ 
		log("sys_access failure\n");
		return -EACCES;
	}

	s_kpathname = kmalloc(sizeof(char)*PATH_MAX, GFP_KERNEL);

	if (s_kpathname == NULL){
		log("s_kpathname failure\n");
		return -ENOMEM;
	}


	if (strncpy_from_user(s_kpathname, pathname, PATH_MAX) < 0){ 
		log("strncpy_from_user failure\n");
		kfree(s_kpathname);
		return -EFAULT;
	}

	log("s_kpathname: %s\n", s_kpathname);

	if (kern_path(s_kpathname, LOOKUP_FOLLOW | LOOKUP_AUTOMOUNT, &s_kpath) != 0){
		log("kern_path failure\n");
		return -EAGAIN;
	}

	file_ind = s_kpath.dentry->d_inode;
	log("s_kpath dentry name: %s \n", s_kpath.dentry->d_name.name);

	if (file_ind == NULL){
		log("file_ind failure\n");
		return -EINVAL;
	}

	/* check if the file is in ext3 */
	if ((file_ind->i_op->get_gps_location) == NULL){
		log("if the file is in ext3 failure\n");
		return -ENODEV;
	}
	
	/* get the gps loc of a file */
	retval = file_ind->i_op->get_gps_location(file_ind, &s_kloc);

	if (copy_to_user(loc, &s_kloc, sizeof(struct gps_location)) != 0){
		log("copy_to_user failure\n");
		kfree(s_kpathname);
		return -EFAULT;
	}

	return retval;
}

void get_gps_data(struct gps_kdata *data)
{
	read_lock(&s_lock);
	
	memcpy (&data->m_lat, s_kdata.m_lat, sizeof(double));
	memcpy (&data->m_lon, s_kdata.m_lon, sizeof(double));
	memcpy (&data->m_acc, s_kdata.m_acc, sizeof(float));
	memcpy (&data->m_age, s_kdata.m_age, sizeof(unsigned long));

	read_unlock(&s_lock);

	log("data m_age is: %d%d%d%d%d\n", data->m_age[0],data->m_age[1],
		data->m_age[2],data->m_age[3]);

	log("data m_lat is: %d%d%d%d%d%d%d%d\n", data->m_lat[0],data->m_lat[1],
		data->m_lat[2],data->m_lat[3],data->m_lat[4],
		data->m_lat[5],data->m_lat[6],data->m_lat[7]);

	log("data m_lon is: %d%d%d%d%d%d%d%d\n", data->m_lon[0],data->m_lon[1],
		data->m_lon[2],data->m_lon[3],data->m_lon[4],
		data->m_lon[5],data->m_lon[6],data->m_lon[7]);

	log("data m_acc is: %d%d%d%d%d%d%d%d\n", data->m_acc[0],data->m_acc[1],
		data->m_acc[2],data->m_acc[3]);
}

