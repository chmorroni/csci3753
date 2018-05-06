#include <linux/kernel.h>
#include <linux/linkage.h>
#include <linux/uaccess.h>

asmlinkage long sys_cs3753_add(const int num1, const int num2, int * ptr_res)
{
  int temp;

  if(!ptr_res)
  {
    return -1;
  }

  printk(KERN_ALERT "Adding %d and %d\n", num1, num2);

  temp = num1 + num2;
  copy_to_user(ptr_res, &temp, sizeof(temp));

  printk(KERN_ALERT "%d + %d = %d\n", num1, num2, temp);

  return 0;
}
