#include <linux/mm.h>
#include <linux/fs.h>
#include <linux/clk.h>
#include <linux/pwm.h>
#include <linux/file.h>
#include <linux/list.h>
#include <linux/gpio.h>
#include <linux/time.h>
#include <linux/sched.h>
#include <linux/delay.h>
#include <linux/module.h>
#include <linux/debugfs.h>
#include <linux/kthread.h>
#include <linux/mfd/core.h>
#include <linux/mempolicy.h>
#include <linux/interrupt.h>
#include <linux/semaphore.h>
#include <linux/mfd/jz_tcu.h>
#include <linux/miscdevice.h>
#include <linux/platform_device.h>
#include <linux/regulator/consumer.h>

#include <soc/irq.h>
#include <soc/base.h>
#include <soc/extal.h>

#include <asm/io.h>
#include <asm/uaccess.h>
#include <asm/cacheflush.h>

#include <mach/platform.h>
#include "motor.h"

static int Motor_speed = 300;
module_param(Motor_speed, int, S_IRUGO);
MODULE_PARM_DESC(Motor_speed, "MOTOR SPEED");

DEF_MOTOR(0);
DEF_MOTOR(1);

#if 1
static unsigned char step_8[8] = {
	0x08,
	0x0c,
	0x04,
	0x06,
	0x02,
	0x03,
	0x01,
	0x09
};
#else
static unsigned char step_8[8] = {
		0x09,
		0x01,
		0x03,
		0x02,
		0x06,
		0x04,
		0x0c,
		0x08
};
#endif
static int motor_step(struct motor_info *info)
{
	int i = info->id;

	if (info->pdata[i]->motor_st1_gpio)
		gpio_direction_output(info->pdata[i]->motor_st1_gpio, step_8[info->run_step] & 0x8);
	if (info->pdata[i]->motor_st2_gpio)
		gpio_direction_output(info->pdata[i]->motor_st2_gpio, step_8[info->run_step] & 0x4);
	if (info->pdata[i]->motor_st3_gpio)
		gpio_direction_output(info->pdata[i]->motor_st3_gpio, step_8[info->run_step] & 0x2);
	if (info->pdata[i]->motor_st4_gpio)
		gpio_direction_output(info->pdata[i]->motor_st4_gpio, step_8[info->run_step] & 0x1);

	return 0;
}


static int motor_set_default(struct motor_info * info)
{
	int index = 0;
	/* 停止定时器 */	
//	printk("XXXXXXXXXXXXXXXXX %s\n", __FUNCTION__);
	jz_tcu_disable_counter(info -> tcu);
	info -> status = MOTOR_MOVE_STOP;
	for (index = 0; index < MOTOR_NUMS; index ++) {
		if (info->pdata[index]->motor_st1_gpio)
			gpio_direction_output(info->pdata[index]->motor_st1_gpio, 0);
		if (info->pdata[index]->motor_st2_gpio)
			gpio_direction_output(info->pdata[index]->motor_st2_gpio, 0);
		if (info->pdata[index]->motor_st3_gpio)
			gpio_direction_output(info->pdata[index]->motor_st3_gpio, 0);
		if (info->pdata[index]->motor_st4_gpio)
			gpio_direction_output(info->pdata[index]->motor_st4_gpio, 0);
	}
	return 0;
}


static irqreturn_t jz_timer_interrupt(int irq, void *dev_id)
{
	struct motor_info * info = (struct motor_info *)dev_id;
	if (info -> cur_steps[info -> direction] == info -> set_steps[info -> direction]) {
		info -> status = MOTOR_MOVE_STOP;
		return IRQ_HANDLED;
	}


	switch(info -> direction) 
	{
		case MOTOR_DIRECTIONAL_UP:	
			if (info -> mattr[MOTOR_POS_UP].total_steps == info -> mattr[MOTOR_POS_UP].cur_steps) {
//				printk("XXXXXXXXXXX reach up edage\n");
				info -> status = MOTOR_MOVE_STOP;
				break;
			}
		
			if (info->cur_steps[info->direction] != info->set_steps[info->direction]) {
				info->run_step = (sizeof(step_8) - 1) - info->cur_steps[info->direction] % sizeof(step_8);
				info->id = 1;
				motor_step(info);
				info->cur_steps[info->direction]++;				
				info -> mattr[MOTOR_POS_UP].cur_steps ++;
			}
			break;

			
		case MOTOR_DIRECTIONAL_DOWN:
			if (info -> mattr[MOTOR_POS_UP].cur_steps == 0) {				
//				printk("XXXXXXXXXXX reach down edage\n");
				info -> status = MOTOR_MOVE_STOP;
				break;
			}

		
			if (info->cur_steps[info->direction] != info->set_steps[info->direction]) {
				info->run_step = info->cur_steps[info->direction] % sizeof(step_8);
				info->id = 1;
				motor_step(info);
				info->cur_steps[info->direction]++;
				info -> mattr[MOTOR_POS_UP].cur_steps --;
			}
			break;

		case MOTOR_DIRECTIONAL_LEFT:
			if (info -> mattr[MOTOR_POS_DOWN].cur_steps == 0) {		
//				printk("XXXXXXXXXXX reach left edage\n");
				info -> status = MOTOR_MOVE_STOP;
				break;
			}
		
			if(info->cur_steps[info->direction] != info->set_steps[info->direction]) {
				info->run_step = info->cur_steps[info->direction] % sizeof(step_8);
				info->id = 0;
				motor_step(info);
				info->cur_steps[info->direction]++;				
				info -> mattr[MOTOR_POS_DOWN].cur_steps --;
			}
			break;
			
		case MOTOR_DIRECTIONAL_RIGHT:
			if (info -> mattr[MOTOR_POS_DOWN].total_steps == info -> mattr[MOTOR_POS_DOWN].cur_steps) {
//				printk("XXXXXXXXXXX reach right edage\n");
				info -> status = MOTOR_MOVE_STOP;
				break;
			}

		
			if(info->cur_steps[info->direction] != info->set_steps[info->direction]) {
				info->run_step = (sizeof(step_8) - 1) - info->cur_steps[info->direction] % sizeof(step_8);
				info->id = 0;
				motor_step(info);
				info->cur_steps[info->direction]++;	
				info -> mattr[MOTOR_POS_DOWN].cur_steps ++;
			}
			break;

			
		default:
			dev_err(info->dev, "unsupport cmd : %d\n", info->direction);
			break;
	}

	return IRQ_HANDLED;
}

static int motor_move(struct motor_info *info, int direction, int steps)
{
	int which_motor = MOTOR_POS_DOWN;	
	if (MOTOR_DIRECTIONAL_UP == direction || MOTOR_DIRECTIONAL_DOWN == direction) {
		which_motor = MOTOR_POS_UP;
	}
	
	info -> direction = direction;
	info -> cur_steps[direction] = 0;
	info -> set_steps[direction] = steps;
//	info -> mattr[which_motor].cur_steps = 0;
	info -> mattr[which_motor].set_steps = steps;
	info -> status = MOTOR_MOVE_RUN;
	return 0;
}

static int motor_speed(struct motor_info *info, int speed)
{
	printk("XXXXXXXXXXXXXXXXXX set motor speed: %d\n", speed);
	if ((speed < info->pdata[0]->min_speed) || (speed > info->pdata[0]->max_speed)) {
		dev_err(info->dev, "speed(%d) set error\n", speed);
		return -1;
	}

	info->speed = speed;
	jz_tcu_set_period(info->tcu, (24000000 / 64 / info->speed));

	return 0;
}

static int motor_attr_init(struct motor_info *info)
{
	/* TODO to be realized */
#if 0
	info->move_is_min = 0;
	info->move_is_max = 0;

	//motor_move(info, MOTOR_DIRECTIONAL_LEFT, 0, info->speed);
	//motor_move(info, MOTOR_DIRECTIONAL_RIGHT, 0, info->speed);
	info->current_steps[0] = info->total_steps[0] / 2;
	info->total_steps[0] = 4096;//motor_move(info, MOTOR_DIRECTIONAL_MAX, 0, info->speed);
#endif
	return 0;
}

static irqreturn_t motor_min_gpio_interrupt(int irq, void *dev_id)
{
	/* TODO to be realized */
#if 0
	int i;
	struct motor_info *info = (struct motor_info *)dev_id;

	for(i = 0; i < 2; i++) {
		if (irq == info->pdata[i]->motor_min_gpio) {
			break;
		}
	}

	info->move_is_min = 1;
	info->move_is_max = 0;
	info->status = MOTOR_MOVE_STOP;
	info->current_steps[i] = 0;
#endif
	return IRQ_HANDLED;
}

static irqreturn_t motor_max_gpio_interrupt(int irq, void *dev_id)
{
	/* TODO to be realized */
#if 0
	int i;
	struct motor_info *info = (struct motor_info *)dev_id;

	for(i = 0; i < 2; i++) {
		if (irq == info->pdata[i]->motor_max_gpio) {
			break;
		}
	}

	info->move_is_min = 0;
	info->move_is_max = 1;
	info->status = MOTOR_MOVE_STOP;
	info->current_steps[i] = info->total_steps[i];
#endif
	return IRQ_HANDLED;
}


static int motor_open(struct inode *inode, struct file *file)
{
	struct miscdevice * dev = file->private_data;
	struct motor_info * info = container_of(dev, struct motor_info, mdev);
	
	jz_tcu_disable_counter(info->tcu);
	motor_set_default(info);
	info->status = MOTOR_MOVE_STOP;
	info->cur_steps[info->direction] = 0;
	info->direction = 0;
	if (!info -> inited) {
		info -> mattr[MOTOR_POS_UP].cur_steps = 5000;
		info -> mattr[MOTOR_POS_UP].total_steps = 10000;
		info -> mattr[MOTOR_POS_DOWN].cur_steps = 5000;
		info -> mattr[MOTOR_POS_DOWN].total_steps = 10000;
	}
	return 0;
}


static int motor_release(struct inode *inode, struct file *file)
{
	return 0;
}


static long motor_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	int ret = 0;
	struct miscdevice *dev = filp->private_data;
	struct motor_info *info = container_of(dev, struct motor_info, mdev);

	switch (cmd) 
	{
		case MOTOR_STOP:
		{
			jz_tcu_disable_counter(info->tcu);
			motor_set_default(info);
			info->status = MOTOR_MOVE_STOP;
			break;
		}
		
		case MOTOR_RESET:
		{
			struct motor_reset_data reset_data;
			ret = copy_from_user(&reset_data, (void __user *)arg, sizeof(struct motor_reset_data));
			if (ret < 0) {
				dev_err(info->dev, "[%s][%d] copy from user error\n", __func__, __LINE__);
				return -EFAULT;
			}

//			printk("XXXXXXXXXXXXXXXXXXXXXXXXXX motor reset\n")
			printk("(%d, %d) ---- (%d, %d)\n",
				reset_data.max_x_steps,
				reset_data.max_y_steps,
				reset_data.cur_x_steps,
				reset_data.cur_y_steps);

			info -> mattr[MOTOR_POS_DOWN].total_steps = reset_data.max_x_steps;
			info -> mattr[MOTOR_POS_UP].total_steps = reset_data.max_y_steps;
			
			info -> mattr[MOTOR_POS_DOWN].cur_steps = reset_data.cur_x_steps;
			info -> mattr[MOTOR_POS_UP].cur_steps = reset_data.cur_y_steps;
			info -> inited = 1;
//			motor_set_default(info);
			break;
		}
		
		case MOTOR_MOVE:
		{
//			printk("XXXXXXXXXXXXXXXXXXXX receive move cmd!\n");
			jz_tcu_enable_counter(info->tcu);
			struct motor_move_st move;
			ret = copy_from_user(&move, (void __user *)arg, sizeof(struct motor_move_st));
			if (ret < 0) {
				dev_err(info->dev, "[%s][%d] copy from user error\n", __func__, __LINE__);
				return -EFAULT;
			}
			motor_move(info, move.motor_directional, move.motor_move_steps);
			break;
		}
		
		case MOTOR_GET_STATUS:
		{
			struct motor_status_st status;
			status.move_is_min = info->move_is_min;
			status.move_is_max = info->move_is_max;
			status.directional_attr = 0;
			status.min_speed = info->pdata[0]->min_speed;
			status.max_speed = info->pdata[0]->max_speed;
			status.cur_speed = info->speed;
			status.current_steps = info->cur_steps[info->direction];
			status.status = info->status;
			status.cur_x_steps = info -> mattr[MOTOR_POS_DOWN].cur_steps;
			status.cur_y_steps = info -> mattr[MOTOR_POS_UP].cur_steps;
			ret = copy_to_user((void __user *)arg, &status, sizeof(struct motor_status_st));
			if (ret < 0) {
				dev_err(info->dev, "[%s][%d] copy to user error\n", __func__, __LINE__);
				return -EFAULT;
			}
#if 0 
			printk("cur steps: (%d, %d)\n", 
				info -> mattr[MOTOR_POS_DOWN].cur_steps,
				info -> mattr[MOTOR_POS_UP].cur_steps);
#endif 
			break;
		}
		
		case MOTOR_SPEED:
		{
			int speed;
			ret = copy_from_user(&speed, (void __user *)arg, sizeof(int));
			if (ret < 0) {
				dev_err(info->dev, "[%s][%d] copy to user error\n", __func__, __LINE__);
				return -EFAULT;
			}

			motor_speed(info, speed);
			break;
		}
		default:
			return -EINVAL;
	}

	return 0;
}

static struct file_operations motor_fops = {
	.open = motor_open,
	.release = motor_release,
	.unlocked_ioctl = motor_ioctl,
};

static int motor_probe(struct platform_device *pdev)
{
	printk("XXXXXXXXXXXXXXXXX build time: %s\n", __TIME__);
	int index = 0;
	int i, j, ret = 0;
	struct motor_info *info;

	info = devm_kzalloc(&pdev->dev, sizeof(struct motor_info), GFP_KERNEL);
	if (!info) {
		ret = -ENOENT;
		dev_err(&pdev->dev, "kzalloc motor_info memery error\n");
		goto error_devm_kzalloc;
	}

	for (index = 0; index < MOTOR_NUMS; index ++) {
		info -> mattr[index].cur_steps = 0;
		info -> mattr[index].set_steps = 0;
		info -> mattr[index].total_steps = Motor_speed;
	}
	
	info->cell = mfd_get_cell(pdev);
	if (!info->cell) {
		ret = -ENOENT;
		dev_err(&pdev->dev, "Failed to get mfd cell for jz_adc_aux!\n");
		goto error_devm_kzalloc;
	}

	info->pdata[0] = &jz_motor0_pdata;
	info->pdata[1] = &jz_motor1_pdata;

	info->speed = Motor_speed;
	printk("motor speed = %d\n", Motor_speed);
	info->dev = &pdev->dev;

	info->tcu = (struct jz_tcu_chn *)info->cell->platform_data;
	info->tcu->irq_type = FULL_IRQ_MODE;
	info->tcu->clk_src = TCU_CLKSRC_EXT;
	info->tcu->prescale = TCU_PRESCALE_64;
	jz_tcu_config_chn(info->tcu);
	jz_tcu_set_period(info->tcu, (24000000 /64/info->speed));
	jz_tcu_start_counter(info->tcu);

	platform_set_drvdata(pdev, info);

	for(i = 0; i < 2; i++) {
		if (info->pdata[i]->motor_min_gpio != -1) {
			gpio_request(info->pdata[i]->motor_min_gpio, "motor_min_gpio");
			ret = request_irq(gpio_to_irq(info->pdata[i]->motor_min_gpio),
					motor_min_gpio_interrupt,
					(info->pdata[i]->motor_gpio_level ?
					 IRQF_TRIGGER_RISING :
					 IRQF_TRIGGER_FALLING)
					| IRQF_DISABLED ,
					"motor_min_gpio", info);
			if (ret) {
				dev_err(&pdev->dev, "request motor_min_gpio error\n");
				goto error_min_gpio;
			}
		}
		
		if (info->pdata[i]->motor_max_gpio != -1) {
			gpio_request(info->pdata[i]->motor_max_gpio, "motor_max_gpio");
			ret = request_irq(gpio_to_irq(info->pdata[i]->motor_max_gpio),
					motor_max_gpio_interrupt,
					(info->pdata[i]->motor_gpio_level ?
					 IRQF_TRIGGER_RISING :
					 IRQF_TRIGGER_FALLING)
					| IRQF_DISABLED ,
					"motor_max_gpio", info);
			if (ret) {
				dev_err(&pdev->dev, "request motor_max_gpio error\n");
				goto error_max_gpio;
			}
		}

		if (info->pdata[i]->motor_st1_gpio != -1) {
			gpio_request(info->pdata[i]->motor_st1_gpio, "motor_st1_gpio");
		}
		if (info->pdata[i]->motor_st2_gpio != -1) {
			gpio_request(info->pdata[i]->motor_st2_gpio, "motor_st2_gpio");
		}
		if (info->pdata[i]->motor_st3_gpio != -1) {
			gpio_request(info->pdata[i]->motor_st3_gpio, "motor_st3_gpio");
		}
		if (info->pdata[i]->motor_st4_gpio != -1) {
			gpio_request(info->pdata[i]->motor_st4_gpio, "motor_st4_gpio");
		}
	}
	
	jz_tcu_disable_counter(info -> tcu);
	info->run_step_irq = platform_get_irq(pdev,0);
	if (info->run_step_irq < 0) {
		ret = info->run_step_irq;
		dev_err(&pdev->dev, "Failed to get platform irq: %d\n", ret);
		goto error_kthread_run;
	}

	ret = request_irq(info->run_step_irq, jz_timer_interrupt, 0, "jz_timer_interrupt", info);
	if (ret) {
		dev_err(&pdev->dev, "Failed to run request_irq() !\n");
		goto error_request_irq;
	}

	info->mdev.minor = MISC_DYNAMIC_MINOR;
	info->mdev.name = "motor";
	info->mdev.fops = &motor_fops;
	ret = misc_register(&info->mdev);
	if (ret < 0) {
		ret = -ENOENT;
		dev_err(&pdev->dev, "misc_register failed\n");
		goto error_misc_register;
	}

	info -> inited = 0;

	return 0;

error_misc_register:
	free_irq(info->run_step_irq, info);
error_request_irq:
//	up(&info->semaphore);
	kthread_stop(info->motor_thread);
error_kthread_run:
	i--;
	if (info->pdata[i]->motor_st1_gpio != -1)
		gpio_free(info->pdata[i]->motor_st1_gpio);

	if (info->pdata[i]->motor_st2_gpio != -1)
		gpio_free(info->pdata[i]->motor_st2_gpio);

	if (info->pdata[i]->motor_st3_gpio != -1)
		gpio_free(info->pdata[i]->motor_st3_gpio);

	if (info->pdata[i]->motor_st4_gpio != -1)
		gpio_free(info->pdata[i]->motor_st4_gpio);
error_max_gpio:
	if (info->pdata[i]->motor_max_gpio != -1) {
		gpio_free(info->pdata[i]->motor_max_gpio);
		free_irq(info->pdata[i]->motor_max_gpio, info);
	}
error_min_gpio:
	if (info->pdata[i]->motor_min_gpio != -1) {
		gpio_free(info->pdata[i]->motor_min_gpio);
		free_irq(info->pdata[i]->motor_min_gpio, info);
	}

	for(j = 0; j < i; i++) {
		if (info->pdata[j]->motor_min_gpio != -1) {
			gpio_free(info->pdata[j]->motor_min_gpio);
			free_irq(info->pdata[j]->motor_min_gpio, info);
		}

		if (info->pdata[j]->motor_max_gpio != -1) {
			gpio_free(info->pdata[j]->motor_max_gpio);
			free_irq(info->pdata[j]->motor_max_gpio, info);
		}

		if (info->pdata[j]->motor_st1_gpio != -1)
			gpio_free(info->pdata[j]->motor_st1_gpio);

		if (info->pdata[j]->motor_st2_gpio != -1)
			gpio_free(info->pdata[j]->motor_st2_gpio);

		if (info->pdata[j]->motor_st3_gpio != -1)
			gpio_free(info->pdata[j]->motor_st3_gpio);

		if (info->pdata[j]->motor_st4_gpio != -1)
			gpio_free(info->pdata[j]->motor_st4_gpio);
	}
error_devm_kzalloc:
	return ret;
}

static int motor_remove(struct platform_device *pdev)
{
	int i;
	struct motor_info *info = platform_get_drvdata(pdev);

	misc_deregister(&info->mdev);
	free_irq(info->run_step_irq, info);

	for(i = 0; i < 2; i++) {
		if (info->pdata[i]->motor_min_gpio != -1) {
			gpio_free(info->pdata[i]->motor_min_gpio);
			free_irq(info->pdata[i]->motor_min_gpio, info);
		}

		if (info->pdata[i]->motor_max_gpio != -1) {
			gpio_free(info->pdata[i]->motor_max_gpio);
			free_irq(info->pdata[i]->motor_max_gpio, info);
		}

		if (info->pdata[i]->motor_st1_gpio != -1)
			gpio_free(info->pdata[i]->motor_st1_gpio);

		if (info->pdata[i]->motor_st2_gpio != -1)
			gpio_free(info->pdata[i]->motor_st2_gpio);

		if (info->pdata[i]->motor_st3_gpio != -1)
			gpio_free(info->pdata[i]->motor_st3_gpio);

		if (info->pdata[i]->motor_st4_gpio != -1)
			gpio_free(info->pdata[i]->motor_st4_gpio);
	}

	return 0;
}

static struct platform_driver motor_driver = {
	.probe = motor_probe,
	.remove = motor_remove,
	.driver = {
		.name	= "tcu_chn2",
		.owner	= THIS_MODULE,
	}
};

static int __init motor_init(void)
{
	return platform_driver_register(&motor_driver);
}

static void __exit motor_exit(void)
{
	platform_driver_unregister(&motor_driver);
}

module_init(motor_init);
module_exit(motor_exit);

MODULE_LICENSE("GPL");
MODULE_VERSION("PTZ ULN2803 20191021");
