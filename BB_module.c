#include <linux/module.h>
#include <linux/kernel.h> /* printk */
#include <linux/interrupt.h> /* irq_request */
#include <linux/irq.h>
#include <asm/uaccess.h> /* copy_to_user */
#include <linux/fs.h> /* file_operations */
#include <linux/gpio.h>
#include <linux/leds.h>
#include <linux/errno.h>
#include <linux/unistd.h>
#include <linux/delay.h>   /* msleep */
#include <linux/timer.h>   /* kernel timer */
#include <linux/slab.h>	   /* kmalloc */

#define NUM_LED 4
#define TIME_STEP  (1*HZ)

#define LED0_GPIO   53    /* USER LED 0     */
#define LED1_GPIO   54    /* USER LED 1     */
#define LED2_GPIO   55    /* USER LED 2     */
#define LED3_GPIO   56    /* USER LED 3     */
#define BUTTON_GPIO 72    /* USER BUTTON S2 */

struct timer_list kerneltimer;
static struct timespec press = {.tv_sec = 0, .tv_nsec = 0};
static struct timespec release = {.tv_sec = 0, .tv_nsec = 0};
static unsigned int button_irq;
static int half = 1;
static int button = 1;
static int count = 0;

void timer_handler (unsigned long arg)
{
	if (half)
	{
		mod_timer(&kerneltimer,  get_jiffies_64() + 8 * TIME_STEP / 10);
		gpio_set_value(LED3_GPIO, (count / 8) % 2);
		gpio_set_value(LED2_GPIO, (count / 4) % 2);
		gpio_set_value(LED1_GPIO, (count / 2) % 2);
		gpio_set_value(LED0_GPIO, count % 2);
	}
	else
	{
		mod_timer(&kerneltimer, get_jiffies_64() + 2 * TIME_STEP / 10);
		gpio_set_value(LED0_GPIO, 0);
		gpio_set_value(LED1_GPIO, 0);
		gpio_set_value(LED2_GPIO, 0);
		gpio_set_value(LED3_GPIO, 0);
	}

	printk(KERN_INFO "%d\n", half);
	half = (half + 1) % 2;
}

static irq_handler_t button_handler(unsigned int irq, void *dev_id, struct pt_regs *regs)
{
	if (button)
	{
		press = current_kernel_time();
		printk(KERN_INFO "%d BUTTON PRESSED\n", count);
	}
	else
	{
		release = current_kernel_time();

		if (!(release.tv_sec - press.tv_sec))
			count++;
		else
			count = 0;

		printk(KERN_INFO "%d BUTTON RELEASED\n", count);
	}

	button = (button + 1) % 2;

	return (irq_handler_t) IRQ_HANDLED;
}

int control_led(void)
{
	int result;

	if (!gpio_is_valid(LED0_GPIO)){
		printk(KERN_INFO "Invalid LED GPIO\n");
		return -1;
	}
	if (!gpio_is_valid(LED1_GPIO)){
		printk(KERN_INFO "Invalid LED GPIO\n");
		return -1;
	}
	if (!gpio_is_valid(LED2_GPIO)){
		printk(KERN_INFO "Invalid LED GPIO\n");
		return -1;
	}
	if (!gpio_is_valid(LED3_GPIO)){
		printk(KERN_INFO "Invalid LED GPIO\n");
		return -1;
	}

	gpio_request(LED0_GPIO, "sysfs");
	gpio_direction_output(LED0_GPIO, 0);
	gpio_export(LED0_GPIO, false);

	gpio_request(LED1_GPIO, "sysfs");
	gpio_direction_output(LED1_GPIO, 0);
	gpio_export(LED1_GPIO, false);

	gpio_request(LED2_GPIO, "sysfs");
	gpio_direction_output(LED2_GPIO, 0);
	gpio_export(LED2_GPIO, false);

	gpio_request(LED3_GPIO, "sysfs");
	gpio_direction_output(LED3_GPIO, 0);
	gpio_export(LED3_GPIO, false);

	gpio_request(BUTTON_GPIO, "sysfs");
	gpio_direction_input(BUTTON_GPIO);
	gpio_set_debounce(BUTTON_GPIO, 10);
	gpio_export(BUTTON_GPIO, false);

	button_irq = gpio_to_irq(BUTTON_GPIO);
	result = request_irq(button_irq, (irq_handler_t) button_handler, IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING, "button_handler", NULL);

	return result;
}

void gpio_exit(void)
{
	gpio_set_value(LED0_GPIO, 0);
	gpio_set_value(LED1_GPIO, 0);
	gpio_set_value(LED2_GPIO, 0);
	gpio_set_value(LED3_GPIO, 0);
	free_irq(button_irq, NULL);

	gpio_unexport(LED0_GPIO);
	gpio_unexport(LED1_GPIO);
	gpio_unexport(LED2_GPIO);
	gpio_unexport(LED3_GPIO);
	gpio_unexport(BUTTON_GPIO);

	gpio_free(LED0_GPIO);
	gpio_free(LED1_GPIO);
	gpio_free(LED2_GPIO);
	gpio_free(LED3_GPIO);
	gpio_free(BUTTON_GPIO);
}

void led_timer_init(void)
{
	init_timer(&kerneltimer);
	kerneltimer.expires = get_jiffies_64();
	kerneltimer.data = 0;
	kerneltimer.function = timer_handler;
	
	add_timer(&kerneltimer);
}

static int __init bb_module_init(void)
{	
	printk("[EE516] Initializing BB module completed.\n");
	if (control_led())
		return -ENODEV;
	led_timer_init();
	return 0;
}

static void __exit bb_module_exit(void)
{		
	printk("[EE516] BB module exit.\n");
	del_timer(&kerneltimer);
	gpio_exit();
}

MODULE_LICENSE("GPL");
module_init(bb_module_init);
module_exit(bb_module_exit);
