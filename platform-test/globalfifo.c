/*
 * 一个简单的字符设备驱动：globalfifo
 * 实现一个全局的FIFO缓冲区，支持多进程并发访问
 *
 * 版权所有 (C) 2014 Barry Song (baohua@kernel.org)
 * 基于GPLv2或更高版本授权
 */

#include <linux/module.h>
#include <linux/types.h>
#include <linux/sched.h>
#include <linux/init.h>
#include <linux/cdev.h>
#include <linux/slab.h>
#include <linux/poll.h>
#include <linux/platform_device.h>
#include <linux/miscdevice.h>
#include <linux/of_device.h>

#define GLOBALFIFO_SIZE 0x1000  // FIFO缓冲区大小4KB
#define FIFO_CLEAR 0x1          // IOCTL清除命令
#define GLOBALFIFO_MAJOR 231    // 主设备号

/* 设备结构体 */
struct globalfifo_dev {
    struct cdev cdev;           // 字符设备结构
    unsigned int current_len;    // 当前数据长度
    unsigned char mem[GLOBALFIFO_SIZE]; // 数据缓冲区
    struct mutex mutex;         // 互斥锁
    wait_queue_head_t r_wait;   // 读等待队列
    wait_queue_head_t w_wait;   // 写等待队列
    struct fasync_struct *async_queue; // 异步通知队列
    struct miscdevice miscdev;  // 杂项设备结构
};

/* 异步通知函数 */
static int globalfifo_fasync(int fd, struct file *filp, int mode)
{
    // 从文件私有数据获取设备结构
    struct globalfifo_dev *dev = container_of(filp->private_data,
        struct globalfifo_dev, miscdev);

    // 调用辅助函数设置异步通知
    return fasync_helper(fd, filp, mode, &dev->async_queue);
}

/* 设备打开函数 */
static int globalfifo_open(struct inode *inode, struct file *filp)
{
    return 0;  // 简单返回成功
}

/* 设备释放函数 */
static int globalfifo_release(struct inode *inode, struct file *filp)
{
    // 移除异步通知设置
    globalfifo_fasync(-1, filp, 0);
    return 0;
}

/* IOCTL控制函数 */
static long globalfifo_ioctl(struct file *filp, unsigned int cmd,
             unsigned long arg)
{
    // 获取设备结构
    struct globalfifo_dev *dev = container_of(filp->private_data,
        struct globalfifo_dev, miscdev);

    switch (cmd) {
    case FIFO_CLEAR:  // 清除FIFO命令
        mutex_lock(&dev->mutex);  // 加锁
        dev->current_len = 0;     // 重置长度
        memset(dev->mem, 0, GLOBALFIFO_SIZE); // 清空缓冲区
        mutex_unlock(&dev->mutex); // 解锁

        printk(KERN_INFO "globalfifo is set to zero\n");
        break;

    default:
        return -EINVAL;  // 不支持的命令
    }
    return 0;
}

/* 轮询函数 */
static unsigned int globalfifo_poll(struct file *filp, poll_table * wait)
{
    unsigned int mask = 0;
    // 获取设备结构
    struct globalfifo_dev *dev = container_of(filp->private_data,
        struct globalfifo_dev, miscdev);

    mutex_lock(&dev->mutex);  // 加锁

    // 将等待队列添加到poll_table
    poll_wait(filp, &dev->r_wait, wait);
    poll_wait(filp, &dev->w_wait, wait);

    // 检查可读状态
    if (dev->current_len != 0) {
        mask |= POLLIN | POLLRDNORM;
    }

    // 检查可写状态
    if (dev->current_len != GLOBALFIFO_SIZE) {
        mask |= POLLOUT | POLLWRNORM;
    }

    mutex_unlock(&dev->mutex); // 解锁
    return mask;
}

/* 读函数 */
static ssize_t globalfifo_read(struct file *filp, char __user *buf,
               size_t count, loff_t *ppos)
{
    int ret;
    // 获取设备结构
    struct globalfifo_dev *dev = container_of(filp->private_data,
        struct globalfifo_dev, miscdev);

    DECLARE_WAITQUEUE(wait, current);  // 定义等待队列项

    mutex_lock(&dev->mutex);  // 加锁
    add_wait_queue(&dev->r_wait, &wait); // 添加到读等待队列

    // 等待直到有数据可读
    while (dev->current_len == 0) {
        if (filp->f_flags & O_NONBLOCK) {  // 非阻塞模式
            ret = -EAGAIN;
            goto out;
        }
        __set_current_state(TASK_INTERRUPTIBLE); // 设置可中断状态
        mutex_unlock(&dev->mutex);  // 解锁

        schedule();  // 调度其他进程
        if (signal_pending(current)) {  // 检查信号
            ret = -ERESTARTSYS;
            goto out2;
        }

        mutex_lock(&dev->mutex);  // 重新加锁
    }

    // 调整读取长度
    if (count > dev->current_len)
        count = dev->current_len;

    // 拷贝数据到用户空间
    if (copy_to_user(buf, dev->mem, count)) {
        ret = -EFAULT;
        goto out;
    } else {
        // 移动剩余数据
        memcpy(dev->mem, dev->mem + count, dev->current_len - count);
        dev->current_len -= count;  // 更新长度
        printk(KERN_INFO "read %d bytes(s),current_len:%d\n", count,
               dev->current_len);

        wake_up_interruptible(&dev->w_wait);  // 唤醒写等待队列

        ret = count;
    }
 out:
    mutex_unlock(&dev->mutex);  // 解锁
 out2:
    remove_wait_queue(&dev->r_wait, &wait);  // 移除等待队列
    set_current_state(TASK_RUNNING);  // 设置运行状态
    return ret;
}

/* 写函数 */
static ssize_t globalfifo_write(struct file *filp, const char __user *buf,
                size_t count, loff_t *ppos)
{
    // 获取设备结构
    struct globalfifo_dev *dev = container_of(filp->private_data,
        struct globalfifo_dev, miscdev);

    int ret;
    DECLARE_WAITQUEUE(wait, current);  // 定义等待队列项

    mutex_lock(&dev->mutex);  // 加锁
    add_wait_queue(&dev->w_wait, &wait); // 添加到写等待队列

    // 等待直到有空间可写
    while (dev->current_len == GLOBALFIFO_SIZE) {
        if (filp->f_flags & O_NONBLOCK) {  // 非阻塞模式
            ret = -EAGAIN;
            goto out;
        }
        __set_current_state(TASK_INTERRUPTIBLE); // 设置可中断状态
        mutex_unlock(&dev->mutex);  // 解锁

        schedule();  // 调度其他进程
        if (signal_pending(current)) {  // 检查信号
            ret = -ERESTARTSYS;
            goto out2;
        }

        mutex_lock(&dev->mutex);  // 重新加锁
    }

    // 调整写入长度
    if (count > GLOBALFIFO_SIZE - dev->current_len)
        count = GLOBALFIFO_SIZE - dev->current_len;

    // 从用户空间拷贝数据
    if (copy_from_user(dev->mem + dev->current_len, buf, count)) {
        ret = -EFAULT;
        goto out;
    } else {
        dev->current_len += count;  // 更新长度
        printk(KERN_INFO "written %d bytes(s),current_len:%d\n", count,
               dev->current_len);

        wake_up_interruptible(&dev->r_wait);  // 唤醒读等待队列

        // 发送异步通知
        if (dev->async_queue) {
            kill_fasync(&dev->async_queue, SIGIO, POLL_IN);
            printk(KERN_DEBUG "%s kill SIGIO\n", __func__);
        }

        ret = count;
    }

 out:
    mutex_unlock(&dev->mutex);  // 解锁
 out2:
    remove_wait_queue(&dev->w_wait, &wait);  // 移除等待队列
    set_current_state(TASK_RUNNING);  // 设置运行状态
    return ret;
}

/* 文件操作结构体 */
static const struct file_operations globalfifo_fops = {
    .owner = THIS_MODULE,      // 模块所有者
    .read = globalfifo_read,   // 读函数
    .write = globalfifo_write, // 写函数
    .unlocked_ioctl = globalfifo_ioctl, // 控制函数
    .poll = globalfifo_poll,   // 轮询函数
    .fasync = globalfifo_fasync, // 异步通知
    .open = globalfifo_open,   // 打开函数
    .release = globalfifo_release, // 释放函数
};

/* 设备探测函数 */
static int globalfifo_probe(struct platform_device *pdev)
{
    struct globalfifo_dev *gl;
    int ret;

    // 分配设备结构内存
    gl = devm_kzalloc(&pdev->dev, sizeof(*gl), GFP_KERNEL);
    if (!gl)
        return -ENOMEM;
    
    // 初始化杂项设备
    gl->miscdev.minor = MISC_DYNAMIC_MINOR; // 动态分配次设备号
    gl->miscdev.name = "globalfifo";        // 设备名称
    gl->miscdev.fops = &globalfifo_fops;    // 文件操作集

    // 初始化互斥锁和等待队列
    mutex_init(&gl->mutex);
    init_waitqueue_head(&gl->r_wait);
    init_waitqueue_head(&gl->w_wait);
    platform_set_drvdata(pdev, gl);  // 设置驱动数据

    // 注册杂项设备
    ret = misc_register(&gl->miscdev);
    if (ret < 0)
        goto err;

    dev_info(&pdev->dev, "globalfifo drv probed\n");
    return 0;
err:
    return ret;
}

/* 设备移除函数 */
static int globalfifo_remove(struct platform_device *pdev)
{
    // 获取设备结构
    struct globalfifo_dev *gl = platform_get_drvdata(pdev);

    // 注销杂项设备
    misc_deregister(&gl->miscdev);

    dev_info(&pdev->dev, "globalfifo drv removed\n");
    return 0;
}

/* 平台驱动结构体 */
static struct platform_driver globalfifo_driver = {
    .driver = {
        .name = "globalfifo",  // 驱动名称
        .owner = THIS_MODULE,  // 模块所有者
    },
    .probe = globalfifo_probe,  // 探测函数
    .remove = globalfifo_remove, // 移除函数
};

/* 模块初始化 */
module_platform_driver(globalfifo_driver);

MODULE_AUTHOR("JIU");
MODULE_LICENSE("GPL v2");
