#include<linux/module.h>

int value=0;
module_param(value,int,S_IRUSR|S_IWUSR);
static int __init hello_init(void)
{
	pr_info("Hello Hi\n");
	return 0;
}

static void __exit hello_CleanUp(void)
{
	pr_info("Bye\n");
}
module_init(hello_init);
module_exit(hello_CleanUp);

MODULE_LICENSE("GPL V2");
MODULE_AUTHOR("ME");
MODULE_DESCRIPTION("A Simple Module");
MODULE_INFO(board,"For a Prog for Host machine");

