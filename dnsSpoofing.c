#include "dnsSpoofing.h"
#include "dnsSpecHeader.h"

/******************************************
 * Input: which function is generating the error
 * Output: VOID                                        
 ******************************************/
static inline void exitError(const char *location)
{
        perror(location);
        exit(1);
}
/******************************************
 * Input: socket Info
 * port number
 * Output: socket number                                        
 *******************************************/
static int setupSocket(struct sockaddr_in *const socketInfo, int port) {

        int socketNum = 0;
        memset((char *) socketInfo, 0, sizeof(*socketInfo));

        socketInfo->sin_family = AF_INET;
        socketInfo->sin_port = htons(port);
        socketInfo->sin_addr.s_addr = htonl(INADDR_ANY);

        if ((socketNum = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
            exitError("Socket");
	}

        if (bind(socketNum , (struct sockaddr*)socketInfo, sizeof(*socketInfo)) == -1)
        {
            exitError("Bind");
        }
        return socketNum;

}
#ifdef DEBUG
/*****************************************
 * prints Header Info for the received DNS msg
******************************************/
static void dumpDnsHeader(dnsHeader_t *const hdrFieldPtr){

        uint16_t flags= ntohs(hdrFieldPtr->flags);

        printf("\nDNS request header Info\n");
        printf("id =  %u\n", ntohs(hdrFieldPtr->id));
        printf("flags =  0x%x\n", (hdrFieldPtr->flags));
        printf(" flags:QR  0x%x\n", (flags & DNS_FLAGS_QR_MASK));
        printf(" flags:OPCODE 0x%x\n", (flags & DNS_FLAGS_OPCODE_MASK));
        printf(" flags:AA 0x%x\n", (flags & DNS_FLAGS_AA));
        printf(" flags:RD 0x%x\n", (flags & DNS_FLAGS_RD));
        printf(" flags:RA 0x%x\n", (flags & DNS_FLAGS_RA));
        printf(" flags:Error 0x%x\n", (flags & DNS_FLAGS_RCODE_MASK));
        printf("question count = %u\n", ntohs(hdrFieldPtr->qdCount));
        printf("answer record count = %u\n", ntohs(hdrFieldPtr->anCount));
        printf("name server record count = %u\n", ntohs(hdrFieldPtr->nsCount));
        printf("additional record count = %u\n", ntohs(hdrFieldPtr->arCount));
}
#endif
/*****************************************
 * calculates the len of the name in request question msg 
 * Input: pointer to the Question field in the request buffer
 * Output: socket number
 *****************************************/
static uint32_t reqQuesNameLen(uint8_t *const reqQuesPtr) {
 
        uint32_t  totalNameLen = 0;
        uint8_t   *nextLen = (uint8_t*)reqQuesPtr;

        while (*nextLen != 0) 
        {
            /* TODO: check the spec for the maxiumun allowed name length, add if */
            totalNameLen += *nextLen + 1;
            nextLen = (uint8_t*)reqQuesPtr + totalNameLen;
#ifdef DEBUG
            printf("next field len = %i, totalNameLen = %i \n", *nextLen,totalNameLen);
#endif
       }
       return totalNameLen;
}
/*****************************************
 * Filling out response hdr
 * Input 1:  pointer to the header of response packet
 * Input 2:  pointer to the header of request/recv packet
 *****************************************/
static void buildDnsRespHdr(dnsHeader_t *const respHdrPtr, dnsHeader_t *const reqHdrPtr) {

         respHdrPtr->id = reqHdrPtr->id;
         respHdrPtr->flags = htons(DNS_FLAGS_QR_REPLY);
         respHdrPtr->qdCount = htons(1);
         respHdrPtr->anCount = htons(1);
         respHdrPtr->nsCount = htons(0);
         respHdrPtr->arCount = htons(0);
}
/*****************************************
 * Filling out response Answer  
 * Input 1: pointer to the Answer field in the response buffer
 *****************************************/
static void buildDnsRespAns(dnsRespTypeA_t *const respAnsFieldPtr) {

         respAnsFieldPtr->compression = htons(0xC00C);
         respAnsFieldPtr->type = htons(0x1);
         respAnsFieldPtr->class = htons(0x1);
         respAnsFieldPtr->ttl = htonl(FIX_TTL);
         respAnsFieldPtr->length = htons(0x4);
         // filling out RDATA field 
         *(uint32_t *)(respAnsFieldPtr + 1) = htonl(IPADD);
}
/*****************************************
 * main
 *****************************************/
int main(void)
{
         struct sockaddr_in  sSocketInfo, cSocketInfo;
         uint32_t socketNum = 0;
	 uint32_t bufLen = 0;
         socklen_t slen = sizeof(sSocketInfo);
	 uint8_t reqBuf[BUF_LEN];
	 uint8_t respBuf[BUF_LEN];

         dnsHeader_t *reqHdrPtr = (dnsHeader_t *)reqBuf;
         dnsHeader_t *respHdrPtr = (dnsHeader_t *)respBuf;
         uint8_t *reqQuesPtr = (reqBuf + sizeof(dnsHeader_t));
         uint8_t *respQuesPtr = (respBuf + sizeof(dnsHeader_t));
         dnsRespTypeA_t *respAnsPtr;
   
         uint32_t totalNameLen;

         socketNum = setupSocket(&sSocketInfo, PORT_NUM);

         while(1)
	 {
                printf("Waiting for data...\n");
#ifdef DEBUG
                memset((char *)reqBuf, 0, BUF_LEN);
                memset((char *)respBuf, 0, BUF_LEN);
                memset((char *) &cSocketInfo, 0, sizeof(cSocketInfo));
#endif
                // receive the request msg from socket
	        slen = sizeof(cSocketInfo);
                if ((bufLen = recvfrom(socketNum, reqBuf, BUF_LEN, 0, (struct sockaddr *)&cSocketInfo, &slen)) == -1)
                {
                    exitError("recvfrom()");
                }
#ifdef DEBUG
                dumpBuffer(reqBuf,bufLen);
                dumpDnsHeader(reqHdrPtr);
#endif             
                if (((ntohs(reqHdrPtr->flags) &  DNS_FLAGS_QR_MASK)  == 0) && (ntohs(reqHdrPtr->qdCount) == 1)) {

		       totalNameLen = reqQuesNameLen(reqQuesPtr);
                       /*add +1 to the length for  the last 0 in the question.name */
                       totalNameLen++;

                       if ((sizeof(dnsHeader_t) + totalNameLen + 4) > bufLen) {
                           printf("Invalid reqPckt: Name length is invalid\n");
                           continue;
                           /* TODO: In multithreading case, take care of synch between client and server */
                       }

                       uint16_t qtype = *(uint16_t *)(reqQuesPtr + totalNameLen + 1);

                       if (qtype) {

                            /* building the response msg */
                            buildDnsRespHdr(respHdrPtr, reqHdrPtr);
                            /* copy name + 4 bytes of  qtype & qclass into respBuf */
                            memcpy (respQuesPtr, reqQuesPtr, totalNameLen + 4);

                            respAnsPtr = (dnsRespTypeA_t *)(&respBuf[sizeof(dnsHeader_t) + totalNameLen + 4]);
	                    buildDnsRespAns(respAnsPtr);

                            /* msglen = hdrLen + questionLen(name + qtypeLen + qClassLen) + responseLen + RDATAlen(4) */
                            bufLen = sizeof(dnsHeader_t) + totalNameLen + 4 + sizeof(dnsRespTypeA_t) + 4;

#ifdef DEBUG
                            dumpBuffer(respBuf,bufLen);
#endif
                            /* response to the request msg */ 
                            if (sendto(socketNum, respBuf, bufLen, 0, (struct sockaddr*)&cSocketInfo, slen) == -1)
		            {
		       	         exitError("sendto()");
	                    }
                      } else { 
                        printf("Invalid Request\n");
                        /* TODO: either call requalr dns or return ERROR to client */
                      }
                } else {
                   printf("Invalid Request\n");
                   /* TODO: either call requalr dns or return ERROR to client */
                }
	}

        close(socketNum);
        return 0;
}
