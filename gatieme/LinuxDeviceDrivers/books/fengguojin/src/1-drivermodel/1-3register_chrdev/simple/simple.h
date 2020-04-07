#ifndef __SIMPLE_H_INCLUDE__
#define __SIMPLE_H_INCLUDE__

#include <linux/ioctl.h> /* needed for the _IOW etc stuff used later */

/********************************************************
 * Macros to help debugging
 ********************************************************/
#undef PDEBUG             /* undef it, just in case */
#ifdef simple_DEBUG
#ifdef __KERNEL__
#    define PDEBUG(fmt, args...) printk( KERN_DEBUG "DEMO: " fmt, ## args)
#else//usr space
#    define PDEBUG(fmt, args...) fprintf(stderr, fmt, ## args)
#endif
#else
#  define PDEBUG(fmt, args...) /* not debugging: nothing */
#endif

#undef PDEBUGG
#define PDEBUGG(fmt, args...) /* nothing: it's a placeholder */

//  �豸��
//  error inserting 'globalmem.ko': -1 Device or resource busy
//  �������ԭ���Ƕ�����豸���г�ͻ,
//  ����ͨ��cat /proc/devices�鿴����Щ�豸�Ż�û��ʹ��
//  ����ֱ����ϵͳ��̬�������豸��
#define simple_MAJOR 224




/////////////////////
//��������
/////////////////////

/*  open���豸�ļ�  */
int simple_open(struct inode *inode, struct file *filp);

/*  close�ͷ��ļ�����  */
int simple_release(struct inode *inode, struct file *filp);


/*  read���豸�ļ�����  */
ssize_t simple_read(struct file *filp, char __user *buf, size_t count,
                   loff_t *f_pos);

/*  writeд�豸�ļ�д����  */
ssize_t simple_write(struct file *filp, const char __user *buf, size_t count,
                    loff_t *f_pos);

/*  �ļ���λ����  */
loff_t  simple_llseek(struct file *filp, loff_t off, int whence);

/*  �豸���Ʋ���  */
int     simple_ioctl(struct inode *inode, struct file *filp,
                    unsigned int cmd, unsigned long arg);


#endif  //  #endif  __SIMPLE_H_INCLUDE__
