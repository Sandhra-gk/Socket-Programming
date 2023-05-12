#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <time.h>

#define PORT 12345
#define BUFFER_SIZE 1024
#define P_SIZE 1030
#define Q_SIZE 256

int sockfd;
struct sockaddr_in servaddr, cliaddr;
socklen_t len = sizeof(cliaddr);	

typedef struct
{
    uint8_t type_of_packet;
    uint32_t sequenceNum;
    char payload[BUFFER_SIZE];
    unsigned char trailer;
    pthread_mutex_t lock;
} Packet;

typedef struct
{
    Packet queue[Q_SIZE];
    int front;
    int rear;
    pthread_mutex_t lock;
} PacketQueue;

PacketQueue packet_queue;
int type_1 = 0; int type_2 = 0;

uint8_t compute_trailer(Packet *packet);
void enqueue(Packet *packet);
Packet dequeue()
{
    //pthread_mutex_lock(&packet_queue.lock);
    packet_queue.front = (packet_queue.front + 1) % Q_SIZE;
    Packet packet = packet_queue.queue[packet_queue.front];
    //pthread_mutex_unlock(&packet_queue.lock);
    return packet;
}


void *error_check_function(void *arg)
{
	
    while (1)
    {
    		Packet packet;
            recvfrom(sockfd, &packet, sizeof(Packet), 0, (struct sockaddr *)&cliaddr, &len);
            if (compute_trailer(&packet) == packet.trailer)
            {
                if(packet.type_of_packet == 1){
                	type_1++;
                }else{
                	type_2++;
                	}	
                enqueue(&packet);
            }
            else
            {
                printf("Error in trailer"); // Indicate that trailer is invalid
            }
        }
    }

void *type1_process_details(void *arg);
void *type2_process_details(void *arg);
void *print_packet_count(void *arg);


void *print_details_1(Packet *packet){
	printf("Packet 1 received\n");
    printf("Packet 1 sequence number : %d\n", packet->sequenceNum);
    printf("Packet 1 payload:");
    for(int i = 0;i< strlen(packet->payload); i++){
    	printf("%c", packet->payload[i]);
    	}
    	printf("\n");
    printf("Packet 1 checksum : %u\n", compute_trailer(packet));
}
   
   
void *print_details_2(Packet *packet){
	printf("Packet 2 received\n");
    printf("Packet 2 sequence number : %d\n", packet->sequenceNum);
    printf("Packet 2 payload:");
    for(int i = 0;i< strlen(packet->payload); i++){
    	printf("%c", packet->payload[i]);
    	}
    	printf("\n");
    printf("Packet 2 checksum : %u\n", compute_trailer(packet));
}
   
   
void create_thread(){
	 pthread_t checker_thread, type_1_thread, type_2_thread, print_thread;
	pthread_create(&checker_thread, NULL, error_check_function, NULL);
	pthread_create(&type_1_thread, NULL, type1_process_details, NULL);
	pthread_create(&type_2_thread, NULL, type2_process_details, NULL);
	pthread_create(&print_thread, NULL, print_packet_count, NULL);
	
	pthread_join(checker_thread, NULL);
	    pthread_join(type_1_thread, NULL);
	    pthread_join(type_2_thread, NULL);
	    pthread_join(print_thread, NULL);

}
int main()
{
    // Create a UDP socket
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&servaddr, 0, sizeof(servaddr));
    memset(&cliaddr, 0, sizeof(cliaddr));

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(PORT);

    // Bind the socket
    if (bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    packet_queue.front=0; packet_queue.rear = -1;
    pthread_mutex_init(&packet_queue.lock, NULL);
    Packet *packet;
            
	create_thread();
	
	    

    close(sockfd);
    return 0;
}

uint8_t  compute_trailer(Packet *packet)
{
    unsigned char trailer = 0;
    uint8_t *packet_contents = (uint8_t *)packet;
    int j;
    for (j= 0; j < P_SIZE - 4; j++)
    {
        trailer = trailer ^ packet_contents[j];
    }
    return trailer;
}

void enqueue(Packet *packet)
{
    pthread_mutex_lock(&packet_queue.lock);
    packet_queue.rear = (packet_queue.rear + 1) % Q_SIZE;
    packet_queue.queue[packet_queue.rear] = *packet;
    // printf("Packet received - seq_num: %d\n", packet->seq_num); // Print sequence number of received packet
    pthread_mutex_unlock(&packet_queue.lock);
}




void *type1_process_details(void *arg)
{
    while (1)
    {
    
    Packet packetx = packet_queue.queue[packet_queue.front];
    pthread_mutex_lock(&packetx.lock);
    if(packetx.type_of_packet == 1){
    print_details_1(&packetx);
    packetx = dequeue();
    pthread_mutex_unlock(&packetx.lock);
    usleep(300*1000);
    }
}
}

void *type2_process_details(void *arg)
{
    while (1)
    {
    Packet packetx = packet_queue.queue[packet_queue.front];
    pthread_mutex_lock(&packetx.lock);
    if(packetx.type_of_packet == 2){
    	print_details_2(&packetx);
    packetx = dequeue();
    pthread_mutex_lock(&packetx.lock);
    usleep(300*1000);
    }
    
}
}


void *print_packet_count(void *arg)
{
   
    while (1)
    {
      
        printf("Total Packets received: Type 1 = %d, Type 2 = %d\n", type_1, type_2);
        usleep(300*1000);
    }
}

