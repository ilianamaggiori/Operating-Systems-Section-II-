/*
 * lunix-chrdev.c
 *
 * Implementation of character devices
 * for Lunix:TNG
 *
 * < Your name here >
 *
 */

#include <linux/mm.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/list.h>
#include <linux/cdev.h>
#include <linux/poll.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/ioctl.h>
#include <linux/types.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/mmzone.h>
#include <linux/vmalloc.h>
#include <linux/spinlock.h>

#include "lunix.h"
#include "lunix-chrdev.h"
#include "lunix-lookup.h"

/*
 * Global data
 */
struct cdev lunix_chrdev_cdev;

/*
 * Just a quick [unlocked] check to see if the cached
 * chrdev state needs to be updated from sensor measurements.
 */
/*
 * Declare a prototype so we can define the "unused" attribute and keep
 * the compiler happy. This function is not yet used, because this helpcode
 * is a stub.
 */
//static int __attribute__((unused)) lunix_chrdev_state_needs_refresh(struct lunix_chrdev_state_struct *);

static int lunix_chrdev_state_needs_refresh(struct lunix_chrdev_state_struct *state)
{
	struct lunix_sensor_struct *sensor=state->sensor;
	
	WARN_ON ( !(sensor = state->sensor));
	if(!sensor){
		debug("refresh: sensor not set\n");
		return -EFAULT;
	}

	/* ? */
	debug("refresh: checking for new data\n");
	printk("refresh: The type is: %d\n",state->type);
	printk("refresh: The last update is %d\n",sensor->msr_data[TEMP]->last_update);
	if(sensor->msr_data[state->type]->last_update>state->buf_timestamp){
		debug("refresh: new data found, leaving\n");
		return 0;
	}
	debug("refresh: no new data found, leaving\n");
	return 1;

	/* The following return is bogus, just for the stub to compile */
	//return 0; /* ? */
}

/*
 * Updates the cached state of a character device
 * based on sensor data. Must be called with the
 * character device state lock held.
 */

static int lunix_chrdev_state_update(struct lunix_chrdev_state_struct *state)
{
	struct lunix_sensor_struct *sensor= state->sensor;
	int result,value_integer,value_fractional;
	uint16_t raw_value;
	long value;
	unsigned long flags;

	if(!sensor){
		debug("update: sensor not set\n");
		return -EFAULT;
	}
	/*
	 * Grab the raw data quickly, hold the
	 * spinlock for as little as possible.
	 */
	/* ? */
	/* Why use spinlocks? See LDD3, p. 119 */
	debug("update: locking spinlock\n");
	spin_lock_irqsave(&sensor->lock,flags);

	/*
	 * Any new data available?
	 */
	/* ? */

	debug("update: calling refresh\n");
	result=lunix_chrdev_state_needs_refresh(state);
	if(result==0) raw_value=sensor->msr_data[state->type]->values[0];

	debug("update: releasing spinlock\n");
	spin_unlock_irqrestore(&sensor->lock,flags);


	/*
	 * Now we can take our time to format them,
	 * holding only the private state semaphore
	 */
	
	/* ? */
	if(result==0){
		debug("update: saving the new timestamp\n");
		state->buf_timestamp=sensor->msr_data[state->type]->last_update;
		debug("update: formatting and saving the new data\n");
		if(state->type==0){
			debug("update: formatting battery measurement\n");
			value=lookup_voltage[raw_value];
			value_integer=value/1000;
			value_fractional=value%1000;
			state->buf_lim=snprintf(state->buf_data, LUNIX_CHRDEV_BUFSZ,"%2d.%03d",value_integer,value_fractional);
		}
		else if(state->type==1){
			debug("update: formatting temperature measurement\n");
			value=lookup_temperature[raw_value];
			value_integer=value/1000;
			value_fractional=value%1000;
			state->buf_lim=snprintf(state->buf_data, LUNIX_CHRDEV_BUFSZ,"%2d.%03d ",value_integer,value_fractional);
		}
		else{
			debug("update: formatting light measurement\n");
			value=lookup_light[raw_value];
			value_integer=value/1000;
			value_fractional=value%1000;
			state->buf_lim=snprintf(state->buf_data, LUNIX_CHRDEV_BUFSZ,"%2d.%03d",value_integer,value_fractional);
		}
		debug("update: leaving\n");
		return 0;
	}

	debug("update:leaving\n");
	return -EAGAIN;
	
}

/*************************************
 * Implementation of file operations
 * for the Lunix character device
 *************************************/

static int lunix_chrdev_open(struct inode *inode, struct file *filp)
{
	/* Declarations */
	/* ? */
	int ret;
	int minor=iminor(inode);
	struct lunix_chrdev_state_struct *state;
	
	
	
	debug("open: entering\n");
	ret = -ENODEV;
	if ((ret = nonseekable_open(inode, filp)) < 0)
		goto out;

	/*
	 * Associate this open file with the relevant sensor based on
	 * the minor number of the device node [/dev/sensor<NO>-<TYPE>].
	 * We can find the type by the last 3 bits of minor number.
	 */
	
	/* Allocate a new Lunix character device private state structure */
	/* ? */
	debug("open: allocating memory for state\n");
	state = kmalloc(sizeof(struct lunix_chrdev_state_struct), GFP_KERNEL);
	if(!state) return -ENOMEM;

	debug("open: associating open file with the relevant sensor");
	state->type=minor%8;
	state->sensor=&lunix_sensors[minor/8];
	debug("open: initializing the buf_timestamp to zero");
	state->buf_timestamp=0;

	debug("read: initializing semaphore\n");
	sema_init(&state->lock, 1);

	debug("open: using filp\n");
	filp->private_data=state;
	debug("open: leaving");
	return 0;

out:
	debug("leaving, with ret = %d\n", ret);
	return ret;
}

static int lunix_chrdev_release(struct inode *inode, struct file *filp)
{
	/* ? */
	kfree(filp->private_data);
	return 0;
}

static long lunix_chrdev_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	/* Why? */
	return -EINVAL;
}

static ssize_t lunix_chrdev_read(struct file *filp, char __user *usrbuf, size_t cnt, loff_t *f_pos)
{

	ssize_t to_read;
	struct lunix_sensor_struct *sensor;
	struct lunix_chrdev_state_struct *state;

	debug("read: setting state\n");
	state = filp->private_data;
	WARN_ON(!state);
	if(!state){
	 	debug("read: state not set\n");
	 	return -EFAULT;
	}
	
	debug("read: setting sensor\n");
	sensor=state->sensor;
	WARN_ON(!sensor);
	if(!sensor) {
	 	debug("read: sensor not set\n");
		return -EFAULT;
	}
	debug("read: sensor set\n");

	/* Lock? */


	debug("read: acquire the semaphore\n");
	if((down_interruptible(&state->lock))!=0) return -EAGAIN;
	
	/*
	 * If the cached character device state needs to be
	 * updated by actual sensor data (i.e. we need to report
	 * on a "fresh" measurement, do so
	 */
	if (*f_pos == 0) {
		debug("read: while: checking for update\n");
		while (lunix_chrdev_state_update(state) == -EAGAIN) {
			/* ? */
			debug("read: releasing the semaphore\n");
			up(&state->lock);
			debug("read: about to sleep\n");
			wait_event_interruptible(sensor->wq,(lunix_chrdev_state_needs_refresh(state)==0));
			debug("read: reacquiring the semaphore\n");
			if((down_interruptible(&state->lock))!=0) return -EAGAIN;
			debug("read: while: checking for update\n");
			/* The process needs to sleep */
			/* See LDD3, page 153 for a hint */
		}
	}
	debug("read: out of the while loop\n");

	/* End of file */
	/* ? */
	

	/* Determine the number of cached bytes to copy to userspace */
	/* ? */
	to_read=min(cnt,(size_t)(state->buf_lim-*f_pos));
	debug("read: about to copy to user\n");
	if(copy_to_user(usrbuf,state->buf_data,to_read)){
		up(&state->lock);
		return -EFAULT;
	}
	*f_pos+=to_read;
	debug("read: releasing the semaphore\n");
	
	/* Auto-rewind on EOF mode? */
	/* ? */
	if(*f_pos==state->buf_lim) {
		*f_pos=0;
	}

	up(&state->lock);
	debug("read: leaving");
	return to_read;
}

static int lunix_chrdev_mmap(struct file *filp, struct vm_area_struct *vma)
{
	return -EINVAL;
}

static struct file_operations lunix_chrdev_fops = 
{
    .owner          = THIS_MODULE,
	.open           = lunix_chrdev_open,
	.release        = lunix_chrdev_release,
	.read           = lunix_chrdev_read,
	.unlocked_ioctl = lunix_chrdev_ioctl,
	.mmap           = lunix_chrdev_mmap
};

int lunix_chrdev_init(void)
{
	/*
	 * Register the character device with the kernel, asking for
	 * a range of minor numbers (number of sensors * 8 measurements / sensor)
	 * beginning with LINUX_CHRDEV_MAJOR:0
	 */
	int ret;
	dev_t dev_no;
	unsigned int lunix_minor_cnt = lunix_sensor_cnt << 3;
	
	debug("initializing character device\n");
	cdev_init(&lunix_chrdev_cdev, &lunix_chrdev_fops); //initialize the cdev structure
	lunix_chrdev_cdev.owner = THIS_MODULE;             //owner field set to THIS_MODULE
	
	dev_no = MKDEV(LUNIX_CHRDEV_MAJOR, 0);
	/* ? */
	/* register_chrdev_region? */

	//allocating minor numbers
	ret=register_chrdev_region(0, lunix_minor_cnt, "/dev/ttyS1"); 

	if (ret < 0) {
		debug("failed to register region, ret = %d\n", ret);
		goto out;
	}	
	/* ? */
	/* cdev_add? */

	//we tell the kernel about the cdev structure that we initialized above
	ret=cdev_add(&lunix_chrdev_cdev, dev_no, lunix_minor_cnt); 
	
	if (ret < 0) {
		debug("failed to add character device\n");
		goto out_with_chrdev_region;
	}
	debug("completed successfully\n");
	return 0;

out_with_chrdev_region:
	unregister_chrdev_region(dev_no, lunix_minor_cnt);
out:
	return ret;
}

void lunix_chrdev_destroy(void)
{
	dev_t dev_no;
	unsigned int lunix_minor_cnt = lunix_sensor_cnt << 3;
		
	debug("entering\n");
	dev_no = MKDEV(LUNIX_CHRDEV_MAJOR, 0);
	cdev_del(&lunix_chrdev_cdev);
	unregister_chrdev_region(dev_no, lunix_minor_cnt);
	debug("leaving\n");
}
