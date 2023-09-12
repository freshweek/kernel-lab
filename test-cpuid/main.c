#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>

#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>

#define SUB_DIR_NAME        "vmx"
#define PROC_FS_NAME        "show_vmx"
#define MODULE_NAME         "show_vmx"


static int proc_open(struct inode *inode, struct file *filp);
static int proc_release(struct inode *inode, struct file *filp);

static struct proc_dir_entry *parent = NULL;
static struct proc_ops proc_ops = {
	.proc_open    = proc_open,
	.proc_read    = seq_read,
	.proc_lseek   = seq_lseek,
	.proc_release = proc_release,
};

static int is_intel_cpu(void)
{
    int a = 0, b, c, d;
    int sig[3] = {
        0x756e6547, 0x6c65746e, 0x49656e69
    };
    asm(
        "cpuid\n\t"
        : "=a"(a), "=b"(b), "=c"(c), "=d"(d)
        : "0"(a)
    );
    return b == sig[0] && c == sig[1] && d == sig[2];
}

static int is_supported_vmx(void)
{
    int a = 1, b, c, d;

    if(! is_intel_cpu()) {
        printk(KERN_ALERT "This is not Intel CPU\n");
        return 0;
    }

    asm(
        "cpuid\n\t"
        : "=a"(a), "=b"(b), "=c"(c), "=d"(d)
        : "0"(a)
    );
    return c & 0x20;
}


static
int proc_show(struct seq_file *m, void *v)
{
    if(is_supported_vmx()) {
		seq_printf(m, "Current CPU support VMX\n");
    } else {
		seq_printf(m, "Current CPU not support VMX\n");
    }

	return 0;
}

static
int proc_open(struct inode *inode, struct file *filp)
{
	/*
	 * the variable passed from proc_create_data() can be extracted
	 * from inode by PED_DATA() macro.
	 * Here, we simply pass it to single_open which will be store as
	 * the `private` field of struct seq_file struct.
	 */
	return single_open(filp, proc_show, NULL);
}

static
int proc_release(struct inode *inode, struct file *filp)
{
	return single_release(inode, filp);
}

static
int __init m_init(void)
{
	printk(KERN_WARNING MODULE_NAME " is loaded\n");

	// parent = proc_mkdir(SUB_DIR_NAME, NULL);
	if (!proc_create(PROC_FS_NAME, 0, parent, &proc_ops))
		return -ENOMEM;

	return 0;
}

static
void __exit m_exit(void)
{
	printk(KERN_WARNING MODULE_NAME " unloaded\n");

	remove_proc_entry(PROC_FS_NAME, parent);
	// remove_proc_entry(SUB_DIR_NAME, NULL);
}

module_init(m_init);
module_exit(m_exit);

MODULE_AUTHOR("Wang, Wenjie");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("A very basic example to test cpuid");
