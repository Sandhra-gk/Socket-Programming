# Socket-Programming
A simulation application that uses UDP sockets for the transmission and reception of packets between a sender and receiver. 
The sender will generate and transmit packets of two different types while the receiver will receive and process the packets. Sender 
packets contain a packet type (1 or 2), sequence number, byte array of 1024 bytes, and a trailer  for the entire packet. Four 
threads will be involved in the receiver process, each with a distinct function. First thread will check for errors in the received packets, 
the second and third threads will process packets of type1 and type2, respectively, and the fourth thread will periodically print out the 
total number of packets received. 
