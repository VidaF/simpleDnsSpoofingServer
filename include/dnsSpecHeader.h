/*flag definations are copied from https://opensource.apple.com/source/netinfo/netinfo-208/common/dns.h.auto.html*/
#define DNS_FLAGS_QR_MASK  0x8000
#define DNS_FLAGS_QR_QUERY 0x0000
#define DNS_FLAGS_QR_REPLY 0x8000

#define DNS_FLAGS_OPCODE_MASK    0x7800
#define DNS_FLAGS_QUERY_STANDARD 0x0000
#define DNS_FLAGS_QUERY_INVERSE  0x0800
#define DNS_FLAGS_QUERY_STATUS   0x1000

#define DNS_FLAGS_AA 0x0400
#define DNS_FLAGS_TC 0x0200
#define DNS_FLAGS_RD 0x0100
#define DNS_FLAGS_RA 0x0080

#define DNS_FLAGS_RCODE_MASK            0x000f
#define DNS_FLAGS_RCODE_NO_ERROR        0x0000
#define DNS_FLAGS_RCODE_FORMAT_ERROR    0x0001
#define DNS_FLAGS_RCODE_SERVER_FAILURE  0x0002
#define DNS_FLAGS_RCODE_NAME_ERROR      0x0003
#define DNS_FLAGS_RCODE_NOT_IMPLEMENTED 0x0004
#define DNS_FLAGS_RCODE_REFUSED         0x0005


/*defined based on DNS spec, https://www.ietf.org/rfc/rfc1035.txt */
typedef struct dnsHeader_s
{
        uint16_t id;        /* Random DNS msg identifier */
        uint16_t flags;     /* Flags: QR:1; OPcode:4; AA:1; TC:1; RD:1;*/
                            /*       RA:1; Z:1; RCODE:4 */
                             
        uint16_t qdCount;   /* Number of Questions */
        uint16_t anCount;   /* Number of Answers */
        uint16_t nsCount;   /* Number of authority records */
        uint16_t arCount;   /* Number of additional records */
} __attribute__((packed)) dnsHeader_t;

typedef struct {
  char *name;        
  uint16_t qtype;  
  uint16_t qclass; 
}  dnsQuestion_t;

typedef struct {
  uint16_t compression;
  uint16_t type;
  uint16_t class;
  uint32_t ttl;
  uint16_t length;
} __attribute__((packed)) dnsRespTypeA_t;

