#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h> 

#define PORT 12345
#define BUFFER_SIZE 1024
#define P_SIZE 1030
#define IP_ADDRESS "127.0.0.1"
int findKey(char c1, char c2, char c3) {
    
    int ascii1 =0;
    int ascii2 =0;
    int ascii3 =0;
    
    if(c1!='\0') {
        ascii1 = (int)c1;
    }
     if(c2!='\0') {
        ascii2 = (int)c2;
    }
     if(c1!='\0') {
        ascii3 = (int)c3;
    }
    
    return ascii1 + ascii2 + ascii3;
    
}
char* getHashCode(char str[]) {
    int length = strlen(str);
    
    char numberDigits[5]="";
    char* rollNum = malloc(5);
    strncat(rollNum, &str[0], 1);
    
    int key1 , key2, key3;
    
    if (length > 8) {
        key1 = findKey(str[0], str[1], str[2]);
        key2 = findKey(str[0], str[2], str[4]);
        key3 = findKey(str[0], str[4], str[8]);
    } else if (length > 4) {
        key1 = findKey(str[0], str[1], str[2]);
        key2 = findKey(str[0], str[2], str[4]);
        key3 = findKey(str[0], str[4], '\0');
    } else if (length > 3 ) {
        key1 = findKey(str[0], str[1], str[2]);
        key2 = findKey(str[0], str[2], '\0');
        key3 = findKey(str[0], '\0', '\0');
    } else if (length == 2) {
        key1 = findKey(str[0], str[1], '\0');
        key2 = findKey(str[0], '\0', '\0');
        key3 = findKey(str[0], '\0', '\0');
    } else if (length == 1) {
        key1 = findKey(str[0], '\0', '\0');
        key2 = findKey(str[0], '\0', '\0');
        key3 = findKey(str[0], '\0', '\0');
    }
    
    int hash1 = (key1%26)%10;
    int hash2 = (key2%26)%10;
    int hash3 = (key3%26)%10;
    
    int number = (((hash1*10)+hash2)*10)+hash3;
    sprintf(numberDigits, "%d", number);
    
    strcat(rollNum , numberDigits);
    
    return rollNum;

}
typedef struct
{
    uint8_t type_of_packet;
    uint32_t sequenceNum;
    char payload[BUFFER_SIZE];
    unsigned char trailer;
} Packet;

uint32_t sequenceNum = 0;
int sockfd;
struct sockaddr_in servaddr; // Create socket
uint8_t compute_trailer(Packet *packet);

void *send_type1_packets(void *arg)
{
    int sockfd = *(int *)arg; // Get the socket file descriptor from the argument
    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);
    servaddr.sin_addr.s_addr = inet_addr(IP_ADDRESS);
    Packet packet;
    struct timespec tspec;

    while (1)
    {
        packet.type_of_packet = 1;
        packet.sequenceNum = sequenceNum++;
        strcpy(packet.payload,"This is type 1 packet");
        packet.trailer = compute_trailer(&packet);

        printf("Sending packet with type=%d, sequenceNum=%d, checksum=%u", packet.type_of_packet, packet.sequenceNum, packet.trailer);
        printf(" and payload =");
        for(int i=0;i<strlen(packet.payload);i++){
        	printf("%c", packet.payload[i]);
        	}
        printf("\n");
        sendto(sockfd, &packet, sizeof(Packet), 0, (const struct sockaddr *)&servaddr, sizeof(servaddr));

        usleep(100*1000);
    }
    return NULL;
}

void *send_type2_packets(void *arg)
{
    int sockfd = *(int *)arg; // Get the socket file descriptor from the argument
    struct sockaddr_in servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);
    servaddr.sin_addr.s_addr = inet_addr(IP_ADDRESS);
    Packet packet;
    // uint32_t sequenceNum = 0;
    struct timespec tspec;

    while (1)
    {
        packet.type_of_packet = 2;
        packet.sequenceNum = sequenceNum++;
        strcpy(packet.payload,"This is type 2 packet");
        packet.trailer = compute_trailer(&packet);

        printf("Sending packet with type=%d, sequenceNum=%d, and checksum=%u ", packet.type_of_packet, packet.sequenceNum, packet.trailer);
        printf("and payload =");
        for(int i=0;i<strlen(packet.payload);i++){
        	printf("%c", packet.payload[i]);
        	}
        printf("\n");
        sendto(sockfd, &packet, sizeof(Packet), 0, (const struct sockaddr *)&servaddr, sizeof(servaddr));

        usleep(150*1000);
    }
    return NULL;
}
void create_thread(){
	pthread_t type1_thread;
    if (pthread_create(&type1_thread, NULL, send_type1_packets, &sockfd) != 0)
    {
        perror("Failed to create type 1 thread");
        exit(EXIT_FAILURE);
    }

    // Create type 2 packets sending thread
    pthread_t type2_thread;
    if (pthread_create(&type2_thread, NULL, send_type2_packets, &sockfd) != 0)
    {
        perror("Failed to create type 2 thread");
        exit(EXIT_FAILURE);
    }

    // Join threads
    pthread_join(type1_thread, NULL);
    pthread_join(type2_thread, NULL);


}
void fill_server_info(){
memset(&servaddr, 0, sizeof(servaddr));

    
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);
    servaddr.sin_addr.s_addr = inet_addr(IP_ADDRESS);


}

int main()
{
    
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }
	// Filling server information
	fill_server_info();
    	char str[10];
	getHashCode(str);
    // Create type 1 and trpe 2 packets sending thread
    create_thread();

    // Close socket
    close(sockfd);

    return 0;
}

uint8_t  compute_trailer(Packet *packet)
{
    unsigned char trailer = 0;
    uint8_t *packet_contents = (uint8_t *)packet;
    for (int i = 0; i < P_SIZE - 4; i++)
    {
        trailer = trailer ^ packet_contents[i];
    }
    return trailer;
}
