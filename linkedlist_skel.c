/*
		Dummy Driver for LinkedList
*/

#include <linux/module.h>
#include <linux/string.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/semaphore.h>
#include <asm/uaccess.h>
#include <linux/init.h>
#include <linux/device.h>

#define MAX_SIZE 10
#define DUMMY_MAJOR_NUMBER 250
#define DUMMY_DEVICE_NAME "DUMMY_DEVICE"

int dummy_open(struct inode *, struct file *);
int dummy_release(struct inode *, struct file *);
ssize_t dummy_read(struct file *, char *, size_t, loff_t *);
ssize_t dummy_write(struct file *, const char *, size_t, loff_t *);
long dummy_ioctl(struct file *, unsigned int, unsigned long);


/* file operation structure */
struct file_operations dummy_fops ={
	open: dummy_open,
	read: dummy_read,
	write: dummy_write,
	release: dummy_release, 
	unlocked_ioctl : dummy_ioctl,
};

char devicename[20];

static struct cdev my_cdev;
static dev_t device_num;   // For device minor number
static struct class *cl;
struct semaphore sem;

struct node{
	int value;
	node *next;
};

node *head;

static int __init dummy_init(void)
{
	int i;

	printk("Dummy Driver : Module Init\n");
	strcpy(devicename, DUMMY_DEVICE_NAME);

	// Allocating device region 
	if (alloc_chrdev_region(&device_num, 0, 1, devicename)){
		return -1;
	}
	if ((cl = class_create(THIS_MODULE, "chardrv" )) == NULL) {
		unregister_chrdev_region(device_num, 1);
		return -1;
	}

	// Device Create == mknod /dev/DUMMY_DEVICE 
	if (device_create(cl, NULL, device_num, NULL, devicename) == NULL)	{
		class_destroy(cl);
		unregister_chrdev_region(device_num, 1);
		return -1;
	}
	
	// Device Init 
	cdev_init(&my_cdev, &dummy_fops);
	if ( cdev_add(&my_cdev, device_num, 1) == -1 ){
		device_destroy(cl, device_num);
		class_destroy(cl);
		unregister_chrdev_region(device_num, 1);
	}

	// Linked list Init
	node *cur = head;
	for (i = 0; i < MAX_SIZE - 1; i++)
	{
		cur->value = -1;
		cur->next = (node*) kmalloc (sizeof(node));
		cur = cur->next;
	}
	cur->value = -1;
	cur->next = NULL;
	
	return 0;
}

static void __exit dummy_exit(void)
{
	printk("Dummy Driver : Clean Up Module\n");
	cdev_del(&my_cdev);
	device_destroy(cl, device_num);
	class_destroy(cl);
	unregister_chrdev_region(MKDEV(DUMMY_MAJOR_NUMBER,0),128);

	// Linked list free
	node *tmp = head;
	for (i = 0; i < MAX_SIZE; i++)
	{
		tmp = tmp->next;
		kfree (head);
		head = tmp;
	}
}

ssize_t dummy_read(struct file *file, char *buffer, size_t length, loff_t *offset)
{
	char val;

	if (copy_to_user(&val, buffer, count))
		return -EFAULT;
	return 0;
}

ssize_t dummy_write(struct file *file, const char *buffer, size_t length, loff_t *offset)
{
	char val;
	int tmp, i;
	node *cur = head;

	//Get user input string to kernel area
	if (copy_from_user(&val, buffer, count))
		return -EFAULT;

	if (val < 48 || val > 57)
	{
		printk (KERN_INFO "wrong number\n");
		return 0;
	}

	tmp = rand() % 100;

	for (i = 0; i < tmp; i++)
		cur = cur->next;

	sema_down (&sem);
	cur->value = val - 48;

	return 0;
}

int dummy_open(struct inode *inode, struct file *file)
{
	printk("Dummy Driver : Open Call\n");
	return 0;
}

int dummy_release(struct inode *inode, struct file *file)
{
	printk("Dummy Driver : Release Call\n");
	return 0;
}

long dummy_ioctl(struct file *file, unsigned int cmd, unsigned long argument)
{
	printk("ioctl");
	return 0;
}


module_init(dummy_init);
module_exit(dummy_exit);

MODULE_DESCRIPTION("Dummy_LinkedList_Driver");
MODULE_LICENSE("GPL");

