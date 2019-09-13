#include<linux/init.h>   
#include<linux/module.h>        
#include<linux/kernel.h>
#include<linux/gpio.h>
#include<linux/interrupt.h>
#include<linux/delay.h>
#include<linux/cdev.h>
#include<linux/uaccess.h>
#include<linux/fs.h>
#include<linux/miscdevice.h>


MODULE_LICENSE("GPL");
MODULE_AUTHOR("SHIN HYUK/SEUNG WON");

#define DEFAULT_GPIO_TRIGGER 4

static int gpio_trigger = DEFAULT_GPIO_TRIGGER;
module_param(gpio_trigger, int, 0644);
MODULE_PARM_DESC(gpio_trigger, "Channel Trigger GPIO");

static DECLARE_WAIT_QUEUE_HEAD(read_wait_queue);

static int isPushed = 0;   

static irq_handler_t irq_handler(int irq, void *arg) {
	local_irq_disable();
	isPushed = 1;
	printk(KERN_ALERT "INTERRUPT %d ", irq);
	local_irq_enable();
	return IRQ_HANDLED;
}


static int button_open(struct inode *pinode, struct file *pfile) {
	int err;

	err = gpio_request(gpio_trigger, THIS_MODULE->name);
	if (err != 0) return err;

	if (gpio_direction_input(gpio_trigger) != 0) {
		gpio_free(gpio_trigger);
		return err;
	}
	gpio_export(gpio_trigger, false);
	err = request_irq(gpio_to_irq(gpio_trigger), (irq_handler_t)irq_handler, IRQF_TRIGGER_RISING, THIS_MODULE->name, NULL);
	printk(KERN_ALERT "GPIO %d > OPEN \n", gpio_trigger);
	
	return err;
}


static int button_release(struct inode *pinode, struct file *pfile) {
	printk(KERN_ALERT "GPIO: %d > RELEASED \n", gpio_trigger);
	gpio_unexport(gpio_trigger);
	gpio_free(gpio_trigger);
	free_irq(gpio_to_irq(gpio_trigger),NULL);
	return 0;
}

static int button_read(struct file *pfile, char __user *buffer, size_t length, loff_t *offset) {
	if (copy_to_user(buffer, &isPushed, sizeof(int)) ){
		printk(KERN_ALERT "copy_to_user(...) failed\n");
		return -EFAULT;
	}
	else printk(KERN_ALERT "copy_to_user(...) succeed\n");
	isPushed = 0;
	return length;
}

static struct file_operations fop = {
	.owner = THIS_MODULE,
	.open = button_open,
	.release = button_release,
	.read = button_read,
};

static struct miscdevice button_driver = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = THIS_MODULE->name,
	.fops = &fop,
};

static int __init button_init(void) {
	int err;
	err = misc_register(&button_driver);
	return 0;
}

static void __exit button_exit(void) {
	printk(KERN_INFO "EXIT \n");
	misc_deregister(&button_driver);
}

module_init(button_init);
module_exit(button_exit);

