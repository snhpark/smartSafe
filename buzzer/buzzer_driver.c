#include<linux/init.h>   
#include<linux/module.h>        
#include<linux/kernel.h>
#include<linux/gpio.h>
#include<linux/interrupt.h>
#include<linux/poll.h>
#include<linux/delay.h>
#include<linux/cdev.h>
#include<linux/uaccess.h>
#include<linux/fs.h>
#include<linux/delay.h>
#include<linux/miscdevice.h>


MODULE_LICENSE("GPL");
MODULE_AUTHOR("SHIN HYUK/SEUNG WON");

#define DEFAULT_GPIO_TRIGGER 13

static int gpio_trigger = DEFAULT_GPIO_TRIGGER;
module_param(gpio_trigger, int, 0644);
MODULE_PARM_DESC(gpio_trigger, "Channel Trigger GPIO");

static DECLARE_WAIT_QUEUE_HEAD(read_wait_queue);

#define PWM_CLKCTL (0xA0/4)
#define PWM_CLKDIV (0xA4/4)

#define PWM_CTL (0x0/4)
#define PWM_RNG2 (0x20/4)
#define PWM_DAT2 (0x24/4)
//141


#define BCM2708_PERI_BASE 0x3F000000

#define GPIO_BASE (BCM2708_PERI_BASE + 0x200000)
#define PWM_BASE (BCM2708_PERI_BASE + 0x20C000)
#define CLK_BASE (BCM2708_PERI_BASE + 0x101000)

#define PAGE_SiZE (4*1024)
//#define BLOCK_SIZE (4*1024)


#define PWEN1 0x0001//0000 0000 0000 0001
#define PWEN2 0x0100//0000 0001 0000 0000
#define MSEN1 0x0080//0000 0000 0100 0000
#define MSEN2 0x8000//0100 0000 0000 0000

static volatile uint32_t *gpio_Vaddr;
static volatile uint32_t *pwm_Vaddr;
static volatile uint32_t *clk_Vaddr;

#define IN_GPIO(g)  (*(gpio_Vaddr + (g/10))) &= ~(7<<((g%10)*3)) 
#define OUT_GPIO(g) (*(gpio_Vaddr + ((g)/10))) |=  (1<<(((g)%10)*3))

#define GPIO_ALT0(g) (*(gpio_Vaddr + (g /10))) |= (4 <<((g %10)*3))

static int clk_div;
static int range;
static int data;

static int msg[3];

void pwm_markspaceMode(void) {
	(*(pwm_Vaddr + PWM_CTL)) = PWEN1 | PWEN2 | MSEN1 | MSEN2;
	udelay(15);
}

void pwm_setRange(int range) {
	(*(pwm_Vaddr + PWM_RNG2)) = range;
	udelay(15);
}

void pwm_setData(int data) {
	(*(pwm_Vaddr + PWM_DAT2)) = data;
	udelay(15);
}

static void pwm_clk_divisor(int clk_div) {
	uint32_t temp;
	temp = *(pwm_Vaddr + PWM_CTL);

	*(pwm_Vaddr + PWM_CTL) = 0;
	*(clk_Vaddr + PWM_CLKCTL) = (0x5A << 24) | 0x01;

	udelay(150);

	while ((*(clk_Vaddr + PWM_CLKCTL) & (0x80)) != 0)
	{
		udelay(1);
	}

	*(clk_Vaddr + PWM_CLKDIV) = (0x5A << 24) | (clk_div << 12);

	*(clk_Vaddr + PWM_CLKCTL) = (0x5A << 24) | 0x11;

	*(pwm_Vaddr + PWM_CTL) = temp;
}



static int buzzer_write(struct file *pfile, const char __user *buffer, size_t length, loff_t *offset) {
	int buffer_size;
	buffer_size = length;

	if (copy_from_user(msg, buffer, buffer_size)) {
		printk(KERN_ALERT "copy_from_user(...) failed\n");
		return -EFAULT;
	}

	switch (msg[0]) {
	case 0:
		pwm_markspaceMode();

		clk_div = msg[1];

		pwm_clk_divisor(clk_div);
		printk(KERN_ALERT "CLK_DIVISOR : %d > PWM\n", clk_div);
		break;

	case 1:
		pwm_markspaceMode();
		range = msg[1];
		pwm_setRange(range);
		printk(KERN_ALERT "range : %d > PWM\n", range);

		break;
		break;

	case 2:
		IN_GPIO(gpio_trigger);
		GPIO_ALT0(gpio_trigger);
		pwm_markspaceMode();

		data = msg[1];
		pwm_setData(data);

		printk(KERN_ALERT "data : %d >PWM\n", data);

		break;

	default:
		printk(KERN_ALERT "WRITE ERROR.\n");
		return -EFAULT;
	}
	return buffer_size;
}


static int buzzer_open(struct inode *pinode, struct file *pfile) {


	printk(KERN_ALERT "GPIO: %d <<< buzzer\n", gpio_trigger);


	gpio_Vaddr = (uint32_t *)ioremap(GPIO_BASE, PAGE_SiZE);

	pwm_Vaddr = (uint32_t *)ioremap(PWM_BASE, PAGE_SiZE);

	clk_Vaddr = (uint32_t *)ioremap(CLK_BASE, PAGE_SiZE);


	return 0;

}


static int buzzer_release(struct inode *pinode, struct file *pfile) {


	gpio_unexport(gpio_trigger);

	gpio_free(gpio_trigger);

	printk(KERN_ALERT "GPIO: %d RELEASED\n", gpio_trigger);
	return 0;
}
static struct file_operations fop = {
	.owner = THIS_MODULE,
	.open = buzzer_open,
	.release = buzzer_release,
	.write = buzzer_write,
};

static struct miscdevice buzzer_driver = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = THIS_MODULE->name,
	.fops = &fop,
};







static int __init buzzer_init(void) {
	int err;

	err = gpio_request(gpio_trigger, THIS_MODULE->name);
	if (err != 0) return err;

	if (gpio_direction_input(gpio_trigger) != 0) {
		gpio_free(gpio_trigger);
		return err;
	}

	err = misc_register(&buzzer_driver);
	printk(KERN_ALERT "buzzer_INIT \n");

	return 0;
}



static void __exit buzzer_exit(void) {
	misc_deregister(&buzzer_driver);

	printk(KERN_INFO "buzzer_EXIT \n");

}

module_init(buzzer_init);
module_exit(buzzer_exit);

