#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <stdlib.h>
#include <unistd.h>            
#include <arpa/inet.h>
#include <string.h>  
#define MSG_LEN 256

char* 	ind="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";   //Index set used to get base64 character from value

struct mail{                      // Structure defined for the message
	int message_type;
	char message[MSG_LEN];
};

int power(int a,int b){           // Helper power function used in encoding
	int temp=1;
	while(b--){
		temp*=a;
	}
	return temp;
}

char * base(char *input,char *encode){	
	int n = strlen(input);
	int i,k,m=0,l,c,temp,j=0;
	int ans[24];
	int val;
	int groups = n/3;             // Total number of groups of 24 bits are groups+1

	for(k=0;k<groups;k++){          // All but last group are taken care of in this for loop
		for(i=0;i<3;i++){       // Take 3 characters at a time = 24 bits
			c=input[3*k+i];
			j=0;
			while(j<=7){            // Binary representation calculated
				temp=c%2;
				ans[8*i+7-j]=temp;
				c=c/2;
				j++;
			}
		}
		j=0;
		for(l=0;l<4;l++){          // 24 bits are grouped in 4 groups of 6 bits
			val=0;
			for(j=0;j<6;j++){
				val+=ans[6*l+5-j]*power(2,j);
			}
			encode[m] = ind[val];      // Value of 6 bits is used to retrive characted from index set
			m++;		
		}
	}

	int endcase1[12]={0};
	int endcase2[18]={0};

	if(n%3==1){	                                // When number of characters are 3n+1
		c=input[3*groups];
		j=0;
		while(j<=7){				// Binary representation of 1 character found
			temp=c%2;			// Padded with 4 zeros
			endcase1[7-j]=temp;
			c=c/2;
			j++;
		}
		j=0;
		for(l=0;l<2;l++){
			val=0;
			for(j=0;j<6;j++){
				val+=endcase1[6*l+5-j]*power(2,j);
			}
			encode[m] = ind[val]; 		// Similarly 2 groups of 6 bits are used to get 2 encoded characters
			m++;		
		}
		encode[m]='=';
		m++;
		encode[m]='='; // Remaining 12 bits in the last group are filled with 2 =
		m++;
	}

	else if(n%3==2){                            // 3n+2 case, 2 characters = 16 bits + 2 padded zeros
		for(i=0;i<2;i++){
			c=input[3*groups+i];
			j=0;
			while(j<=7){
				temp=c%2;
				endcase2[8*i+7-j]=temp;
				c=c/2;
				j++;
			}
		}
		j=0;
		for(l=0;l<3;l++){
			val=0;
			for(j=0;j<6;j++){
				val+=endcase2[6*l+5-j]*power(2,j);
			}
			
			encode[m] = ind[val];
			m++;		
		}	
		encode[m]='=';      // Last 6bits are compensated by 1 =
		m++;
	}
	encode[m] = '\0';
}



void error(char *msg){ // Helper error function
	perror(msg);
	exit(0);
}


int main(int argc, char *argv[]){
	int sockfd, portno, n;
	struct sockaddr_in serv_addr;
	struct hostent *server;
	char* buffer=(char *)malloc(MSG_LEN*sizeof(char));   // Allocating buffer space for socket

	if (argc < 3) {
		fprintf(stderr,"usage %s hostname port\n", argv[0]);
		exit(0);
	}
	
	portno = atoi(argv[2]);
	sockfd = socket(AF_INET, SOCK_STREAM, 0); //Create socket for client
	
	if (sockfd < 0){ 
		error("ERROR opening socket");
	}
	
	server = gethostbyname(argv[1]); // Get server address
	
	if (server == NULL) {
		fprintf(stderr,"ERROR, no such host\n");
		exit(0);
	}
	
	bzero((char *) &serv_addr, sizeof(serv_addr));  // Filling in various fields of serv_addr
	serv_addr.sin_family = AF_INET;
	bcopy((char *)server->h_addr,(char *)&serv_addr.sin_addr.s_addr,server->h_length);
	serv_addr.sin_port = htons(portno);
	
	if (connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0){ //Initiate connection to server
		error("ERROR connecting");
	}
	
	struct mail msg;
	char temp[MSG_LEN-1];
	bzero(buffer,MSG_LEN);
	n = read(sockfd,buffer,MSG_LEN);
	
	if (n < 0){
		error("ERROR reading from socket");
	}

	printf("%s\n",buffer);

	while(1){
		bzero(buffer,MSG_LEN);
		printf("Please enter the message : \n");  
		fgets(buffer,MSG_LEN,stdin);   // Get message to be sent from stdin
		buffer[strlen(buffer)-1]='\0'; // Delete carriage return character
		msg.message_type=buffer[0] - '0'; //First byte is message type
		strcpy(msg.message,buffer+1);

		if(msg.message_type==1){ //Send Message
			char *to_send=(char*)malloc((sizeof(char)*MSG_LEN*4+6)/3);
			to_send[0]='1';
			base(buffer+1,to_send+1);      //Obtain encoded message in to_send
			n = write(sockfd,to_send,strlen(to_send)); 
			free(to_send);
		
			if (n < 0){
				error("ERROR writing to socket");  
			}	
		
			bzero(buffer,MSG_LEN);    
			n = read(sockfd,buffer,MSG_LEN);	
			printf("%s\n",buffer);
		}
		
		else if(msg.message_type==3){ //Close connection
			n = write(sockfd,buffer,strlen(buffer));

			if (n < 0){
				error("ERROR writing to socket");
			}

			break;	
		}
		
		else{
			printf("Start the input with either 1 or 3\n");
			continue;
		}
	}
	close(sockfd);     // Close socket
	free(buffer);	// Deallocate buffer space
	return 0;
}


