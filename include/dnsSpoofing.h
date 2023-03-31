#include <stdio.h>       
#include <string.h> 
#include <stdlib.h> 
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <unistd.h>


#define DEBUG 0


#define BUF_LEN   2048     //socket buffer length
#define PORT_NUM  53     //socket port number, defult 53
#define IPADD     0x06060606
#define FIX_TTL   0xaaaa


#ifdef DEBUG
/*****************************************
 * dump memory
 * Input 1: memory address
 * Input 2: dump length
 ******************************************/
void dumpBuffer(uint8_t *const buf, uint32_t len){
    
    printf("\n");
    for(uint32_t i = 0; i <= len; i++) {
        printf("0x%x,  ", buf[i]);
       //printf("buf[%i] = 0x%x  ",i, buf[i]);
    }
    printf("\n");
}
#endif
