#include "stdio.h"
#include "stdint.h"
#include <fcntl.h>
#include<sys/ioctl.h>
#include <unistd.h>

#define WR_ONLY _IOW('a','a',int32_t)
#define RD_ONLY _IOR('a','b',int32_t)
#define RD_WR   _IOWR('a','c',int32_t)


int main()
{
    int32_t fd,value,number;

    fd=open("/dev/Example_Device",O_RDWR);
    if(fd<0) 
    {
        printf("Couldn't open File\n"); 
        return 0;
    }

    printf("Enter the value to be Written\n");
    scanf("%d",&number);
    printf("Value Written is %d\n",number);
    ioctl(fd,WR_ONLY,&number);

    
    ioctl(fd,RD_ONLY,&value);
    printf("Read Value from Driver %d\n",value);


    close(fd);
}