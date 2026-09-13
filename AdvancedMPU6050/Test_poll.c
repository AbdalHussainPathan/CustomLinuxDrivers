#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>      /* open(), O_RDONLY */
#include <unistd.h>     /* read(), close() */
#include <poll.h>       /* poll(), struct pollfd, POLLIN */   
//To compile : gcc Test_poll.c -o Poll_App
//always use sudo ./Poll_App
int main()
{
    int fd = open("/dev/MPU_6050", O_RDONLY);
    if (fd < 0) 
    {
        perror("open");
        return 1;
    }
    struct pollfd pfd = { .fd = fd, .events = POLLIN};
    int ret=0;
    short count=0;
    char buf[128]={0};
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