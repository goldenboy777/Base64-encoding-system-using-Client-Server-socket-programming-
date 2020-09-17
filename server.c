#include <stdio.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>    		//close  
#include <arpa/inet.h>    //close  
#include <netinet/in.h>  
#include <sys/time.h> //FD_SET, FD_ISSET, FD_ZERO macros  
#include <string.h>
#include <stdlib.h>	
#define MSG_LEN 256


struct mail{
	int message_type;
	char message[MSG_LEN];   
};


void string_to_bin(char * input,int *bin,int n){ //Function to get binary representation of encoded string
	for(int i=n;i<n+4;i++){
		int temp;
		
		if(input[i]>='A' && input[i]<='Z'){   // First get their index value in the index set used in encoder
			temp=input[i]-'A';
		}
		
		else if(input[i]>='a' && input[i]<='z'){
			temp=input[i]-'a'+26;
		}
		
		else if(input[i]>='0' && input[i]<='9'){
			temp=input[i]-'0'+52;
		}
		
		else if(input[i]=='+'){
			temp=62;
		}
		
		else if(input[i]=='/'){
			temp=63;
		}
		
		else{
			temp=0;
		}
		
		for(int j=5;j>=0;j--){ // Convert to binary
			bin[j+(i-n)*6]=temp%2;
			temp=temp/2;
		}
	}
}


void convert_to_string(char *output,int* bin,int s,int type){ // Taking a group of 24 bits and converting into 3 characters
	for(int i=0;i<3;i++){
		int temp=0;
		int val=1;
		
		for(int j=7;j>=0;j--){
			temp+=val*bin[j+i*8];
			val=val*2;
		}
		
		temp=temp-97;
		output[s+i]='a'+temp;
	}
}


void decode(char *input,int n,char *output){
	int bin[24];
	if(n%4){
		printf("Wrong input\n");
		return;
	}

	int temp=(3*n)/4;
	if(input[n-2]=='='){	// When input(un-encoded) is of form 3n+1
		temp=temp-2;
	}

	else if(input[n-1]=='='){ // When input(un-encoded) is of form 3n+2
		temp=temp-1;
	}
	
	else{
		temp=3*n/4;
	}

	for(int i=0;i<n;i+=4){	// Converting group of 24 bits to binary then string
		string_to_bin(input,bin,i);
		convert_to_string(output,bin,3*i/4,0);
	}

	output[temp]='\0';
}




void error(char *msg){
		perror(msg);
		exit(1);
}

int main(int argc, char *argv[]){
	int max_sd , max_clients = 30 ;  
	int client_socket[max_clients] , probing, i , sd;    
	int sockfd, newsockfd, portno, clilen;
	char* buffer=(char *)malloc(MSG_LEN*sizeof(char)); // Server buffer
	struct sockaddr_in serv_addr, cli_addr;
	int n;
	fd_set readfds; 	// List of socket fds

	for (i = 0; i < max_clients; i++){     // client_socket is a bitmap denoting occupied/non-occupied sockets
		client_socket[i] = 0;   
	}

	if (argc < 2){
		fprintf(stderr,"ERROR, no port provided\n");
		exit(1);
	}

	sockfd = socket(AF_INET, SOCK_STREAM, 0);	// Socket alloted
	
	if (sockfd < 0) {
		error("ERROR opening socket");
	}
	
	bzero((char *) &serv_addr, sizeof(serv_addr)); // Fields of serv_addr filled before binding
	portno = atoi(argv[1]);
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(portno);
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	
	if (bind(sockfd, (struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0){
		error("ERROR on binding");
	}
	
	listen(sockfd,5); // Actively listening for connection from client
	struct mail msg;

	while(1){
		FD_ZERO(&readfds);
		FD_SET(sockfd, &readfds);
		max_sd = sockfd;
		
		for ( i = 0 ; i < max_clients ; i++){
			  
			sd = client_socket[i];   
			  

			if(sd > 0){   	// Only sd which are in use/ value =1 are added
				FD_SET( sd , &readfds);
			}   
			  

			if(sd > max_sd){  // Max_sd needed for select later
				max_sd = sd;
			}   
		}
			 
		probing = select( max_sd + 1 , &readfds , NULL , NULL , NULL);  // Select gets activated if any sockfd in readfs is active

		if ((probing < 0)){   
			error("select error");   
		}   	
					 
		if (FD_ISSET(sockfd, &readfds)){ // Checks if there is activity on main socket, indicating new connection
			clilen = sizeof(cli_addr);
			newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen); // Accept the incoming connection

			if (newsockfd < 0){
				error("ERROR on accept");
			}
			
			printf("New connection , socket fd is %d , ip is : %s , port : %d\n" , newsockfd , inet_ntoa(cli_addr.sin_addr) , ntohs(cli_addr.sin_port)); 
			n = write(newsockfd,"Welcome to team",15);
			if (n < 0){
				error("ERROR writing to socket");
			}
					
			//puts("Welcome message sent successfully\n");   
			
			for (i = 0; i < max_clients; i++){
				// Add new connection into forst non-zero entry of client_socket  
				if( client_socket[i] == 0 ){
					client_socket[i] = newsockfd;   
					printf("Adding to list of sockets as %d\n" , i);   
					break;   
				}   
			}   
		}
		 

		// Else its some IO operation on some other socket 

		for (i = 0; i < max_clients; i++){
			sd = client_socket[i];  
			if (FD_ISSET( sd , &readfds)){ // Check if that particular sd has activity
				bzero(buffer,MSG_LEN);
				n = read(sd,buffer,MSG_LEN);
				
				if (n < 0){
					error("ERROR reading from socket");
				}

				//Closing case
				if (n == 0){   
                     			getpeername(sd , (struct sockaddr*)&cli_addr ,(socklen_t*)&cli_addr);
                    			printf("Host disconnected , ip %s , port %d\n",inet_ntoa(cli_addr.sin_addr),ntohs(cli_addr.sin_port));            
                    			close( sd );   // Close socket
                    			client_socket[i] = 0;   // Clear bitmap
                		}    
     			

				msg.message_type=buffer[0]-'0'; // Get message type from first byte
				strcpy(msg.message,buffer+1);
			
				if(msg.message_type==1){
					printf("Here is the message : %s\n",msg.message);
					char output[3*strlen(msg.message)/4];
					decode(msg.message,strlen(msg.message),output); // Decode the base64 message
					printf("Decoded output: %s\n",output);
					bzero(buffer,MSG_LEN);
					buffer[0]='2';
					strcpy(buffer+1,"Here is the Acknowledgement for your previous message");
					n = write(sd,buffer,MSG_LEN); // Send back an ACK
					
					if (n < 0){ 
						error("ERROR writing to socket");
					}

				}
				
				else if(msg.message_type==3){ // 3 type is for connection closing
					getpeername(sd , (struct sockaddr*)&cli_addr ,(socklen_t*)&cli_addr);
					printf("Host disconnected , ip %s , port %d \n",inet_ntoa(cli_addr.sin_addr),ntohs(cli_addr.sin_port));                        
					close( sd );   // Close socket
					client_socket[i] = 0;   // Clear bitmap
				}
			}    
		}
	}    
	free(buffer); // Deallocate server buffer
	return 0; 
}
