
#include<linux/init.h>
#include<linux/module.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/gfp.h>
#include <linux/uaccess.h>

#define DRIVER_MAJOR_NUM (61)
#define DRIVER_NAME ("simple_character_device")
#define BUFFER_SIZE (32)

int simple_char_init(void);
void simple_char_exit(void);
int simple_char_open(struct inode * inode, struct file * file);
int simple_char_close(struct inode * inode, struct file * file);
loff_t simple_char_llseek(struct file * file, loff_t offset, int whence);
ssize_t simple_char_read(struct file * file, char __user * user_buf, size_t len, loff_t * curr_pos);
ssize_t simple_char_write(struct file * file, const char __user * user_buf, size_t len, loff_t * curr_pos);

static struct file_operations simple_char_file_ops = 
{
  .llseek = simple_char_llseek,
  .read = simple_char_read,
  .write = simple_char_write,
  .open = simple_char_open,
  .release = simple_char_close,
};

static char * device_buffer = NULL;
static int times_opened = 0;
static int write_pos = 0;

int simple_char_init(void)
{
  if( register_chrdev(DRIVER_MAJOR_NUM, DRIVER_NAME, &simple_char_file_ops) )
  {
    printk(KERN_ALERT "simple_char_dev: register failed\n");
    return -1;
  }

  device_buffer = (char *)kmalloc(BUFFER_SIZE, GFP_KERNEL);
  if( !device_buffer )
  {
    printk(KERN_ALERT "simple_char_dev: kmalloc failed\n");
    return -1;
  }
  memset(device_buffer, 0, BUFFER_SIZE);

  printk(KERN_ALERT "simple_char_dev: initialized\n");

	return 0;
}

void simple_char_exit(void)
{
  unregister_chrdev(DRIVER_MAJOR_NUM, DRIVER_NAME);

  kfree( (void *)device_buffer );

  printk(KERN_ALERT "simple_char_dev: exited");
}

int simple_char_open(struct inode * inode, struct file * file)
{
  printk(KERN_ALERT "simple_char_dev: opening - opened %d times\n", ++times_opened);
  return 0;
}

int simple_char_close(struct inode * inode, struct file * file)
{
  printk(KERN_ALERT "simple_char_dev: closing - opened %d times\n", times_opened);
  return 0;
}

loff_t simple_char_llseek(struct file * file, loff_t offset, int whence)
{
  if(whence == SEEK_SET && offset < BUFFER_SIZE && offset >= 0)
  {
    write_pos = offset;
    printk(KERN_ALERT "simple_char_dev: seek to offset %d\n", (int)offset);
    return 0;
  }
  if(whence == SEEK_CUR && write_pos + offset < BUFFER_SIZE && write_pos + offset >= 0)
  {
    write_pos += offset;
    printk(KERN_ALERT "simple_char_dev: seek to offset %d from current location", (int)offset);
    return 0;
  }
  if(whence == SEEK_END && offset <= BUFFER_SIZE && offset > 0);
  {
    write_pos = BUFFER_SIZE - offset;
    printk(KERN_ALERT "simple_char_dev: seek to offset %d from end", (int)offset);
    return 0;
  }
  printk(KERN_ALERT "simple_char_dev: invalid seek");
  return -1;
}

ssize_t simple_char_read(struct file * file, char __user * user_buf, size_t len, loff_t * curr_pos)
{
  int read_len = len < (BUFFER_SIZE - *curr_pos) ? len : BUFFER_SIZE - *curr_pos;
  copy_to_user(&user_buf[*curr_pos], device_buffer, read_len);
  *curr_pos += read_len;
  printk(KERN_ALERT "simple_char_dev: read %d bytes\n", read_len);
  return read_len;
}

ssize_t simple_char_write(struct file * file, const char __user * user_buf, size_t len, loff_t * curr_pos)
{
  if(write_pos + len > BUFFER_SIZE)
  {
    int write_len = BUFFER_SIZE - write_pos;
    copy_from_user(&device_buffer[write_pos], user_buf, write_len);
    write_pos = 0;
    simple_char_write(file, &user_buf[write_len], len - write_len, curr_pos);
    printk(KERN_ALERT "simple_char_dev: wrote %d bytes and overflowed\n", write_len);
  }
  else
  {
    copy_from_user(&device_buffer[write_pos], user_buf, len);
    write_pos += len;
    printk(KERN_ALERT "simple_char_dev: wrote %d bytes\n", (int)len);
  }
  return len;
}

module_init(simple_char_init);
module_exit(simple_char_exit);
