#include <linux/device.h>
#include <linux/fs.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/miscdevice.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/version.h>
#include <linux/uaccess.h>

#define DEFAULT_GPIO_TRIGGER  16 // Pin 12 on Raspberry Pi P1 connector.

// Minimal interval between two read() in microsec
#define MIN_INTERVAL          10 * 1000


//------------------- Module parameters -------------------------------------

static int gpio_trigger = DEFAULT_GPIO_TRIGGER;
module_param(gpio_trigger, int, 0644);
MODULE_PARM_DESC(gpio_trigger, "Sound Trigger GPIO.");

static DECLARE_WAIT_QUEUE_HEAD(read_wait_queue);



// ------------------ Driver private data type ------------------------------

struct sound_sensor_struct {
    struct timeval last_timestamp;
    int            value;
    spinlock_t     spinlock;
} g_sound_sensor;



// ------------------ Driver private methods -------------------------------

static int sound_sensor_open (struct inode * ind, struct file * filp)
{
  printk(KERN_DEBUG "%s: open() - dummy method.\n", THIS_MODULE->name);
  return 0;
}

static int sound_sensor_release (struct inode * ind,  struct file * filp)
{
  printk(KERN_DEBUG "%s: release() - dummy method.\n", THIS_MODULE->name);
  return 0;
}



static ssize_t sound_sensor_read(struct file * filp, char * __user buffer, size_t length, loff_t * offset)
{
		int lg;
		char kbuffer[16];
		unsigned long irqmsk;
		int val;
		int ret;
	
		ret = wait_event_interruptible(read_wait_queue, g_sound_sensor.value != 0);
		
		if (ret < 0) {
				printk(KERN_DEBUG "%s: read() - wake up by signal.\n", THIS_MODULE->name);
          return -ERESTARTSYS;
		}

		spin_lock_irqsave(& (g_sound_sensor.spinlock), irqmsk);
		val = g_sound_sensor.value;
		snprintf(kbuffer, 16, "%d\n", val);
		g_sound_sensor.value = 0;
		spin_unlock_irqrestore(& (g_sound_sensor.spinlock), irqmsk);

		lg = strlen(kbuffer);

		lg -= (*offset);
		
		if(lg <= 0)
				return 0;

		if(lg > length)
				lg = length;

		if(copy_to_user(buffer, kbuffer+(*offset), lg)!= 0)
				return -EFAULT;

		// No more data in the buffer.
		memset(kbuffer, 0, sizeof(kbuffer)); 

		// (*offset) += lg; 
		(*offset) = 0;
	
		return lg;
}



static ssize_t sound_sensor_write(struct file * filp, const char * __user buffer, size_t length, loff_t * offset)
{
		printk(KERN_DEBUG "%s: write() has been executed.\n", THIS_MODULE->name);

		return length;
}



static irqreturn_t gpio_trigger_handler(int irq, void * arg)
{
		struct sound_sensor_struct *sensor = arg;
		struct timeval timestamp;
		long int evt_intval;
	
	//	printk("%d\n",sensor->value);
		
		do_gettimeofday(& timestamp);

		evt_intval = MIN_INTERVAL + 1;

		if (sensor == NULL)
				return -IRQ_NONE;

		if((sensor->last_timestamp.tv_sec  != 0)||(sensor->last_timestamp.tv_usec != 0)) {
	  evt_intval  = timestamp.tv_sec - sensor->last_timestamp.tv_sec;
	  evt_intval *= 1000000;  // In microsec.
	  evt_intval += timestamp.tv_usec - sensor->last_timestamp.tv_usec;
		}

		printk(KERN_DEBUG "%s: interrupt handler interval=%ld.\n", THIS_MODULE->name, evt_intval);

		if(evt_intval > MIN_INTERVAL) {
				spin_lock(& sensor->spinlock);
		
				if(gpio_get_value(gpio_trigger)==1)
						sensor->value = 1; 
				else
						sensor->value = 0;

				spin_unlock(& sensor->spinlock);

				wake_up_interruptible(&read_wait_queue);
		}
		else{
				printk(KERN_DEBUG "%s: interrupt handler has dropped one event.\n", THIS_MODULE->name);
		}

		sensor->last_timestamp = timestamp;

		printk(KERN_DEBUG "%s: GPIO pin %d (as input) has been triggered.\n", THIS_MODULE->name, gpio_trigger);

    return IRQ_HANDLED;
}



// ------------------ Driver private global data ----------------------------

static struct file_operations sound_sensor_fops = {
    .owner   =  THIS_MODULE,
    .open    =  sound_sensor_open,
    .read    =  sound_sensor_read,
    .release =  sound_sensor_release,
    .write   =  sound_sensor_write,
};



static struct miscdevice sound_sensor_driver = {
        .minor          = MISC_DYNAMIC_MINOR,
        .name           = THIS_MODULE->name,
        .fops           = & sound_sensor_fops,
};



// ------------------ Driver init and exit methods --------------------------

static int __init sound_sensor_init (void)
{
		int err;

		spin_lock_init(& (g_sound_sensor.spinlock));
		g_sound_sensor.value = 0;
	
		// Reserve GPIO TRIGGER.
		err = gpio_request(gpio_trigger, THIS_MODULE->name);
		if(err != 0)
				return err;

		// Set GPIO Trigger as input.
		if(gpio_direction_input(gpio_trigger) != 0) {
				gpio_free(gpio_trigger);
				return err;
		}

		// Install IRQ handlers.
		err = request_irq(gpio_to_irq(gpio_trigger), gpio_trigger_handler,
	                  IRQF_SHARED | IRQF_TRIGGER_RISING,
	                  THIS_MODULE->name, & g_sound_sensor);
		if(err != 0) {
				gpio_free(gpio_trigger);
				return err;
		}

		printk(KERN_INFO "%s: init() - GPIO pin %d has been configured as input.\n", THIS_MODULE->name, gpio_trigger);

		// Install user space char interface.
		err = misc_register(& sound_sensor_driver);
		return err;
}



void __exit sound_sensor_exit (void)
{
		misc_deregister(& sound_sensor_driver);
	
		free_irq(gpio_to_irq(gpio_trigger), & g_sound_sensor);
		gpio_free(gpio_trigger);

		printk(KERN_INFO "%s: exit() - successfully unloaded.\n", THIS_MODULE->name);
}



module_init(sound_sensor_init);
module_exit(sound_sensor_exit);



MODULE_LICENSE("GPL");
