#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include <fcntl.h>
#include <stdbool.h>
#include <math.h>
#include <stdio.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h> // read(), write(), close()
#include "jpeglib.h"
#include <png.h>
#include "img2base64.c"


#define MAX 8000
#define PORT 1717
#define SA struct sockaddr


// instalation 
// git clone https://github.com/nothings/stb.git
// sudo cp stb/stb_image.h /usr/local/include/
// sudo cp stb/stb_image_write.h /usr/local/include/


void clasify_image(unsigned char *image_name, int connfd){

	char buff[MAX];
	bzero(buff, MAX);
	int width, height, channels;
    unsigned char *image = stbi_load(image_name, &width, &height, &channels, 0);

    if (image == NULL) {
        strncpy(buff, "Error loading image \n", sizeof(buff) - 1); 
		write(connfd, buff, sizeof(buff));
    }

    int redCount = 0;
    int greenCount = 0;
    int blueCount = 0;

    // Analize image pixel's
    for (int i = 0; i < width * height * channels; i += channels) {
        unsigned char red = image[i];
        unsigned char green = image[i + 1];
        unsigned char blue = image[i + 2];

        redCount += red;
        greenCount += green;
        blueCount += blue;
    }

    stbi_image_free(image);

    // Clasifify
    if (redCount > greenCount && redCount > blueCount) {
		char folder_name[100] = "Images_t2so/rojo/";
		strcat(folder_name,image_name);
		if(!stbi_write_png(folder_name, width, height, channels, image, width * channels)){
			printf("Error saving the image.\n");
			strncpy(buff, "Error saving the image.\n", sizeof(buff) - 1); 
			write(connfd, buff, sizeof(buff));
		}else{
			printf("Predominant color : RED.\n");
			strncpy(buff, "Predominant color : RED.\n", sizeof(buff) - 1); 
			write(connfd, buff, sizeof(buff));
		}
        
    } else if (greenCount > redCount && greenCount > blueCount) {
		char folder_name[100] = "Images_t2so/verde/";
		strcat(folder_name,image_name);
		if(!stbi_write_png(folder_name, width, height, channels, image, width * channels)){
			printf("Error saving the image.\n");
			strncpy(buff, "Error saving the image.\n", sizeof(buff) - 1); 
			write(connfd, buff, sizeof(buff));
		}else{
			printf("Predominant color : GREEN.\n");
			strncpy(buff, "Predominant color : RED.\n", sizeof(buff) - 1); 
			write(connfd, buff, sizeof(buff));
		}

    } else {
		char folder_name[100] = "Images_t2so/azul/";
		strcat(folder_name,image_name);
		if(!stbi_write_png(folder_name, width, height, channels, image, width * channels)){
			printf("Error saving the image.\n");
			strncpy(buff, "Error saving the image.\n", sizeof(buff) - 1); 
			write(connfd, buff, sizeof(buff));
		}else{
			printf("Predominant color : BLUE.\n");
			strncpy(buff, "Predominant color : BLUE.\n", sizeof(buff) - 1); 
			write(connfd, buff, sizeof(buff));
		}
        
    }
}

void histogram_equalization(unsigned char *image, int width, int height) {
    // Calculate first histogram
    int histogram[256] = {0};
    for (int i = 0; i < width * height; i++) {
        histogram[image[i]]++;
    }

    // Calculate acumulate distribution (CDF)
    int cdf[256];
    cdf[0] = histogram[0];
    for (int i = 1; i < 256; i++) {
        cdf[i] = cdf[i - 1] + histogram[i];
    }

    // Apply ecualization to the image
    for (int i = 0; i < width * height*3; i++) {
        image[i] = (unsigned char)(((float)cdf[image[i]] - cdf[0]) / (width * height - 1) * 255.0f + 0.5f);
    }
}

void ecualizeImage(char* imagePath, int connfd){
	char buff[MAX];
	bzero(buff, MAX);
    int width, height, channels;
    unsigned char *image = stbi_load(imagePath, &width, &height, &channels, 0);

    if (image == NULL) {

		strncpy(buff, "Error loading image \n", sizeof(buff) - 1); 
		write(connfd, buff, sizeof(buff));
       
    }

    // Aplica la ecualización del histograma a la imagen
    histogram_equalization(image, width, height);
    // Guarda la imagen resultante
    if (!stbi_write_png("Images_t2so/filtered/output_filtered.jpg", width, height, channels, image, width * channels)) {

		strncpy(buff, "Error saving the output image \n", sizeof(buff) - 1); 
		write(connfd, buff, sizeof(buff));
    
        
    }

    // Limpia la memoria
    stbi_image_free(image);

	strncpy(buff, "Image processed with succes! \n", sizeof(buff) - 1); 
	write(connfd, buff, sizeof(buff));
}



// Function designed for chat between client and server.
void func(int connfd)
{
	char buff[MAX];
	int n;
	// infinite loop for chat
	for (;;) {
		bzero(buff, MAX);

		// read the message from client and copy it in buffer
		read(connfd, buff, sizeof(buff));

		// print buffer which contains the client contents
		printf("From client: %s", buff);
		

		// Functions calls
		if(strncmp(buff,"eqimage", 7) == 0){
			bzero(buff, MAX);
			n = 0;
			strncpy(buff, "¿Whats the file name? \n", sizeof(buff) - 1); 
			// and send that buffer to client
			write(connfd, buff, sizeof(buff));

			// Reading image name
			int lng;
			for (;;) {

				bzero(buff,MAX);
				read(connfd, buff, sizeof(buff));
				lng = strlen(buff);

				if((strncmp(buff,"\n",1)==0)){
					bzero(buff, MAX);
					printf("Invalid Input\n");
					strncpy(buff, "Invalid Input \n", sizeof(buff) - 1); 
					write(connfd, buff, sizeof(buff));
					break;
				}

				buff[strlen(buff) - 1] = '\0';
				printf("Is image: %s\n", buff);
				decode(buff);
				ecualizeImage("decoded_image.jpg", connfd);
				clasify_image("decoded_image.jpg",connfd);
				break;

			}


		}
		// if msg contains "Exit" then server exit and chat ended.
		else if (strncmp("exit", buff, 4) == 0) {
			bzero(buff, MAX);
			n = 0;
			strncpy(buff, "exit", sizeof(buff) - 1); 
			// and send that buffer to client
			write(connfd, buff, sizeof(buff));
			break;
		}

		else{

			bzero(buff, MAX);
			n = 0;
			strncpy(buff, "That command doesnt exist \n", sizeof(buff) - 1); 
			// and send that buffer to client
			write(connfd, buff, sizeof(buff));

		}
	}
}




// Driver function
int main()
{
	int sockfd, connfd, len;
	struct sockaddr_in servaddr, cli;

	// socket create and verification
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd == -1) {
		printf("socket creation failed...\n");
		exit(0);
	}
	else
		printf("Socket successfully created..\n");
	bzero(&servaddr, sizeof(servaddr));

	// assign IP, PORT
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(PORT);

	// Binding newly created socket to given IP and verification
	if ((bind(sockfd, (SA*)&servaddr, sizeof(servaddr))) != 0) {
		printf("socket bind failed...\n");
		exit(0);
	}
	else
		printf("Socket successfully binded..\n");

	// Now server is ready to listen and verification
	if ((listen(sockfd, 5)) != 0) {
		printf("Listen failed...\n");
		exit(0);
	}
	else
		printf("Server listening..\n");
	len = sizeof(cli);

	// Accept the data packet from client and verification
	connfd = accept(sockfd, (SA*)&cli, &len);
	if (connfd < 0) {
		printf("server accept failed...\n");
		exit(0);
	}
	else
		printf("server accept the client...\n");

	// Function for chatting between client and server
	func(connfd);

	// After chatting close the socket
	close(sockfd);
}
