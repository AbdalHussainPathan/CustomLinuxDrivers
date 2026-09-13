#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <fcntl.h>      /* open(), O_RDONLY */
#include <unistd.h>     /* read(), close() */
#include <poll.h>       /* poll(), struct pollfd, POLLIN */   
#include<sys/ioctl.h>
#include <errno.h>

#define MPU_IOC_MAGIC  'M'
#define MPU_IOC_SET_TIMEOUT   _IOW(MPU_IOC_MAGIC, 1, uint32_t)
#define MPU_IOC_RESET_BUF  _IO(MPU_IOC_MAGIC, 2)
//To compile : gcc Test_poll.c -o Poll_App
//always use sudo ./Poll_App
int main(int num_args, char *args[])
{
    int ret=0;
    short count=0;
    char buf[128]={0};
    errno = 0;
    if(num_args>2)
    {
        printf("Invalid args");
        return 0;
    }
    long val = strtol(args[1], NULL, 10);
    if (errno != 0 || val == 0) {
        // handle error: not a valid number
    }
    int TimeoutValue = (int) val;
    printf("TimeoutValue is %d\n",TimeoutValue);
    int fd = open("/dev/MPU_6050", O_RDONLY);
    if (fd < 0) 
    {
        perror("open");
        return 1;
    }
    struct pollfd pfd = { .fd = fd, .events = POLLIN};
    ioctl(fd,MPU_IOC_SET_TIMEOUT,TimeoutValue);
    while(1)
    {
        ret = poll(&pfd, 1, 500);  /* wait up to 5s */
        if (ret > 0 && (pfd.revents & POLLIN)) {
            read(fd, buf, sizeof(buf));  /* safe — data is ready */
            while(buf[count]!='\0')
            {
               printf("%c", buf[count++]);
            }
            count=0; 
        }
        else
        {
            printf("No Data Arrived!\n");
        }  
    } 
    return 0;
}