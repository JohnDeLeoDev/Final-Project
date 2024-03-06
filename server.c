// Written by: John DeLeo
// WPI CS5008, Fall 2023
// Final Programming Project

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/time.h>
#include <time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>

// Add client to access log
void add_to_access_log(char *client_ip) {
    int current_year;
    int current_month;
    int current_day;
    int current_hour;
    int current_minute;
    int current_second;

    time_t current_time;
    time(&current_time);
    struct tm *local_time = localtime(&current_time);

    current_year = local_time->tm_year + 1900;
    current_month = local_time->tm_mon + 1;
    current_day = local_time->tm_mday;
    current_hour = local_time->tm_hour;
    current_minute = local_time->tm_min;
    current_second = local_time->tm_sec;

    FILE *access_log;
    access_log = fopen("./access.log", "a");
    fprintf(access_log, "%d-%02d-%02d %02d:%02d:%02d - %s\n", current_year, current_month, current_day, current_hour, current_minute, current_second, client_ip);
    fclose(access_log);
}

// Check for client in access log
int check_access_log(char *client_ip, int attempts, int per_time) {
    int current_year;
    int current_month;
    int current_day;
    int current_hour;
    int current_minute;
    int current_second;

    time_t current_time;
    time(&current_time);
    struct tm *local_time = localtime(&current_time);

    current_year = local_time->tm_year + 1900;
    current_month = local_time->tm_mon + 1;
    current_day = local_time->tm_mday;
    current_hour = local_time->tm_hour;
    current_minute = local_time->tm_min;
    current_second = local_time->tm_sec;

    FILE *access_log;
    access_log = fopen("./access.log", "r");
    char access_log_line[512];
    int line_number = 0;
    int attempts_count = 0;

    while (fgets(access_log_line, sizeof(access_log_line), access_log)) {
        line_number++;
        if (strstr(access_log_line, client_ip)) {
            int pos, len;
            char access_log_year[5];
            strncpy(access_log_year, access_log_line, 4);
            access_log_year[4] = '\0';

            char access_log_month[3]; 
            pos = 5;
            len = 2;
            strncpy(access_log_month, access_log_line + pos, len);
            access_log_month[2] = '\0';

            char access_log_day[3]; 
            pos = 8;
            len = 2;
            strncpy(access_log_day, access_log_line + pos, len);
            access_log_day[2] = '\0';

            char access_log_hour[3]; 
            pos = 11;
            len = 2;
            strncpy(access_log_hour, access_log_line + pos, len);
            access_log_hour[2] = '\0';

            char access_log_minute[3]; 
            pos = 14;
            len = 2;
            strncpy(access_log_minute, access_log_line + pos, len);
            access_log_minute[2] = '\0';

            char access_log_second[3]; 
            pos = 17;
            len = 2;
            strncpy(access_log_second, access_log_line + pos, len);
            access_log_second[2] = '\0';

            int access_log_year_int = atoi(access_log_year);
            int access_log_month_int = atoi(access_log_month);
            int access_log_day_int = atoi(access_log_day);
            int access_log_hour_int = atoi(access_log_hour);
            int access_log_minute_int = atoi(access_log_minute);
            int access_log_second_int = atoi(access_log_second);

            int time_difference = (current_hour - access_log_hour_int)*3600 + (current_minute - access_log_minute_int)*60 + (current_second - access_log_second_int);

            if (time_difference < per_time) {
                attempts_count++;
            }
        }
    }
    fclose(access_log);
    
    if (attempts_count > attempts) {
        return 1;
    } else {
        return 0;
    }

}

// Write to log
void write_to_log(char *message) {
    int current_year;
    int current_month;
    int current_day;
    int current_hour;
    int current_minute;
    int current_second;

    time_t current_time;
    time(&current_time);
    struct tm *local_time = localtime(&current_time);

    current_year = local_time->tm_year + 1900;
    current_month = local_time->tm_mon + 1;
    current_day = local_time->tm_mday;
    current_hour = local_time->tm_hour;
    current_minute = local_time->tm_min;
    current_second = local_time->tm_sec;

    FILE *log_file;
    log_file = fopen("./server.log", "a");
    fprintf(log_file, "%d-%02d-%02d %02d:%02d:%02d - %s\n", current_year, current_month, current_day, current_hour, current_minute, current_second, message);
    fclose(log_file);
}

// Cleanup QR code
void cleanup_qr_code(char *file_name) {
    remove(file_name);
}

// Cleanup decoded QR code
void cleanup_decoded_qr(char *file_name) {
    remove(file_name);
}

int main(int argc, char *argv[]) {
    int response_code = 0;
    char server_message[256];
    long file_size_limit = 4294967295; // 4GB
    int port = 2012;
    int attempts = 2;
    int per_time = 60;
    int max_users = 3;
    struct timeval time_out;
    time_out.tv_sec = 80;
    time_out.tv_usec = 0;
    int server_socket, bind_status;
    int client_socket;
    struct sockaddr_in server_address, client_address;
    char log_message[512];
    socklen_t client_address_length;
    pid_t childpid;
    int client_count = 0;
    int client_id = 0;


    // Handles exit of program so that all sockets are closed and logs are updated.
    void handle_exit(int sig) {
        sprintf(log_message, "Server shutting down.");
        write_to_log(log_message);
        close(client_socket);
        sprintf(log_message, "Client socket closed.");
        write_to_log(log_message);
        close(server_socket);
        sprintf(log_message, "Server socket closed.");
        write_to_log(log_message);
        sprintf(log_message, "Server shut down.");
        write_to_log(log_message);
        sprintf(log_message, "###########################################################");
        write_to_log(log_message);
        printf("\nServer shut down.\n");
        exit(0);
    }


    // Logging server start.
    sprintf(log_message, "###########################################################");
    write_to_log(log_message);
    sprintf(log_message, "Server started by user. PID: %d", getpid());
    write_to_log(log_message);


    // Signal handler for SIGINT that triggers calling of handle_exit function.
    signal(SIGINT, handle_exit);



    // Logging arguments passed to server.
    if (argc == 1) {
        sprintf(log_message, "No arguments passed to server.");
        write_to_log(log_message);
    } else {
        sprintf(log_message, "Arguments passed to server.");
        write_to_log(log_message);
    }
    for (int i = 1; i < argc; i++) {
        sprintf(log_message, "Argument %d: %s", i, argv[i]);
        write_to_log(log_message);
    }


    // Startup message.
    sprintf(log_message, "Server starting.");
    write_to_log(log_message);
    printf("Server starting... Exit at any time with CTRL+C.\n");


    // Server configuration.
    if (argc > 1) {
        for (int i = 1; i < argc; i++) {
            if (strcmp(argv[i], "PORT") == 0) {
                port = atoi(argv[i+1]);
            }
            if (strcmp(argv[i], "RATE") == 0) {
                attempts = atoi(argv[i+1]);
                per_time = atoi(argv[i+2]);
            }
            if (strcmp(argv[i], "MAXUSERS") == 0) {
                max_users = atoi(argv[i+1]);
            }
            if (strcmp(argv[i], "TIMEOUT") == 0) {
                time_out.tv_sec = atoi(argv[i+1]);
            } 
        }
    } 
    sprintf(log_message, "Listening port set to %d.", port);
    write_to_log(log_message);
    sprintf(log_message, "Rate limiting set to %d requests per user per %d seconds.", attempts, per_time);
    write_to_log(log_message);
    sprintf(log_message, "Maximum number of concurrent users set to %d.", max_users);
    write_to_log(log_message);
    sprintf(log_message, "Time-out connections set to %ld seconds.", time_out.tv_sec);
    write_to_log(log_message);
    sprintf(log_message, "Server configuration complete.");
    write_to_log(log_message);


    // Server socket creation and binding.
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    sprintf(log_message, "Server socket created.");
    write_to_log(log_message);
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port); 
    server_address.sin_addr.s_addr = INADDR_ANY;
    sprintf(log_message, "Server address defined.");
    write_to_log(log_message);
    bind_status = bind(server_socket, (struct sockaddr *) &server_address, sizeof(server_address));



    // Error handling for server socket binding.
    if (bind_status < 0) {
        sprintf(log_message, "Server socket failed to bind.");
        write_to_log(log_message);
        printf("Server socket failed to bind.\n");
        exit(0);
    } else {
        sprintf(log_message, "Server socket binded.");
        write_to_log(log_message);
    }
    


    // Server socket listening.
    listen(server_socket, max_users);
    sprintf(log_message, "Server socket listening.");
    write_to_log(log_message);
    client_address_length = sizeof(struct sockaddr_in);



    // Server started message.
    printf("Server started on port %d.\n", port);
    printf("Waiting for client connection...\n");
    


    // Loop to constantly listen for client connections.
    while (1) {


        // Checking if maximum number of concurrent users has been reached.
        if (client_count < max_users) {
            client_socket = accept(server_socket, (struct sockaddr *) &client_address, &client_address_length);
            client_count++;
            sprintf(log_message, "Server socket accepted.");
            write_to_log(log_message);

            setsockopt(client_socket, SOL_SOCKET, SO_RCVTIMEO, (const char *) &time_out, sizeof(time_out));
            sprintf(log_message, "Server socket timeout applied to client socket.");
            write_to_log(log_message);
            client_id++;
            childpid = fork();



            // Checking if error occurred while forking.
            if (childpid < 0) {
                sprintf(log_message, "Server socket failed to fork.");
                write_to_log(log_message);
                exit(0);

            } else if (childpid == 0) {
                sprintf(log_message, "Server socket forked.");
                write_to_log(log_message);
                char *client_ip = inet_ntoa(client_address.sin_addr);
                add_to_access_log(client_ip);
                sprintf(log_message, "New client connection from %s", client_ip);
                write_to_log(log_message);

                int rate_limit_violated = check_access_log(client_ip, attempts, per_time);



                // Checking if client has violated rate limit.
                if (rate_limit_violated == 1) {
                    response_code = 3;
                    sprintf(log_message, "%s - Client violated rate limit.", client_ip);
                    write_to_log(log_message);

                    send(client_socket, &response_code, sizeof(int), 0);
                    sprintf(log_message, "%s - Response code sent to client.", client_ip);
                    write_to_log(log_message);

                    strcpy(server_message, "Rate limit exceeded.");
                    send(client_socket, &server_message, sizeof(server_message), 0);

                    close(client_socket);
                    sprintf(log_message, "%s - Client socket closed.", client_ip);
                    write_to_log(log_message);
                    printf("Connection attempted from %s, rate limit exceeded.\n", client_ip);
                    exit(0);
                }        


                // Logging client IP and ID.
                sprintf(log_message, "%s - Client ID: %d", client_ip, client_id);
                write_to_log(log_message);


                // Logging client IP, ID, and port.
                int client_port = ntohs(client_address.sin_port);
                sprintf(log_message, "%s - %d - Client port: %d", client_ip, client_id, client_port);
                write_to_log(log_message); 


                // Logging start time of connection
                time_t start_time;
                time(&start_time);
                sprintf(log_message, "%s - %d - Timer started.", client_ip, client_id);
                write_to_log(log_message);


                // Sending INITIAL response code to client.
                send(client_socket, &response_code, sizeof(int), 0);
                sprintf(log_message, "%s - %d - Response code sent to client.", client_ip, client_id);
                send(client_socket, "You have reached the server.", sizeof("You have reached the server."), 0);
                sprintf(log_message, "%s - %d - Welcome message sent to client.", client_ip, client_id);
                write_to_log(log_message);


                // Allows me to implement a time-out for the client socket.
                setsockopt(client_socket, SOL_SOCKET, SO_RCVTIMEO, (const char *) &time_out, sizeof(time_out));


                // Receiving QR code size from client.
                int qr_code_size;
                if (recv(client_socket, &qr_code_size, sizeof(int), 0) < 0) {
                    sprintf(log_message, "%s - %d - Client timed out.", client_ip, client_id);
                    write_to_log(log_message);
                    response_code = 2;

                    send(client_socket, &response_code, sizeof(int), 0);
                    sprintf(log_message, "%s - %d - Response code sent to client.", client_ip, client_id);
                    write_to_log(log_message);

                    strcpy(server_message, "Client timed out.");
                    send(client_socket, &server_message, sizeof(server_message), 0);

                    close(client_socket);
                    sprintf(log_message, "%s - %d - Client socket closed.", client_ip, client_id);
                    write_to_log(log_message);
                    exit(0);
                }
                

                // Checking if QR code size exceeds file size limit.
                if (qr_code_size > file_size_limit) {
                    sprintf(log_message, "%s - %d - QR code size exceeds file size limit.", client_ip, client_id);
                    write_to_log(log_message);
                    response_code = 1;

                    send(client_socket, &response_code, sizeof(int), 0);
                    sprintf(log_message, "%s - %d - Response code sent to client.", client_ip, client_id);
                    write_to_log(log_message);

                    strcpy(server_message, "QR code size exceeds file size limit.");
                    send(client_socket, &server_message, sizeof(server_message), 0);

                    close(client_socket);
                    sprintf(log_message, "%s - %d - Client socket closed.", client_ip, client_id);
                    write_to_log(log_message);
                    exit(0);
                }
                

                // Logging QR code size received from client.
                sprintf(log_message, "%s - %d - QR code size received from client, %d bytes.", client_ip, client_id, qr_code_size);
                write_to_log(log_message);

                // buffer to store QR code from client
                char buffer[qr_code_size];


                // handles QR code to local file system that is unique based on client_id
                FILE *received_file;
                char file_name[256];
                sprintf(file_name, "received_%d.png", client_id);
                received_file = fopen(file_name, "wb");


                // Allows me to implement a time-out for the client socket.
                setsockopt(client_socket, SOL_SOCKET, SO_RCVTIMEO, (const char *) &time_out, sizeof(time_out));


                // Receiving ACTUAL QR code file from client.
                int bytesReceived = 0;
                int totalBytesReceived = 0;


                // Loop to receive QR code from client. If client times out, server sends response code 2 to client and closes client socket.
                while (totalBytesReceived < qr_code_size) {
                    bytesReceived = recv(client_socket, buffer, qr_code_size, 0);


                    // Timeout handling.
                    if (bytesReceived < 0) {
                        sprintf(log_message, "%s - %d - Client timed out.", client_ip, client_id);
                        write_to_log(log_message);
                        response_code = 2;

                        send(client_socket, &response_code, sizeof(int), 0);
                        sprintf(log_message, "%s - %d - Response code sent to client.", client_ip, client_id);
                        write_to_log(log_message);

                        strcpy(server_message, "Client timed out.");
                        send(client_socket, &server_message, sizeof(server_message), 0);

                        close(client_socket);
                        sprintf(log_message, "%s - %d - Client socket closed.", client_ip, client_id);
                        write_to_log(log_message);
                        exit(0);
                    }

                    // Otherwise continue receiving QR code from client.
                    fwrite(buffer, 1, bytesReceived, received_file);
                    totalBytesReceived += bytesReceived;
                    if (bytesReceived == 0) {
                        break;
                    }
                }


                // Closes file and logs that QR code has been received.
                fclose(received_file);
                sprintf(log_message, "%s - %d - QR code received.", client_ip, client_id);
                write_to_log(log_message);


                // Decoding QR code with Java library through command line.
                int qr_code_decode_response;
                char decoded_qr[256];
                char decoded_qr_file_name[512];
                sprintf(decoded_qr_file_name, "decoded_qr_%d.txt", client_id);
                char system_command[1024];
                sprintf(system_command, "java -cp javase.jar:core.jar com.google.zxing.client.j2se.CommandLineRunner %s > %s", file_name, decoded_qr_file_name);

                // Executes command line command to decode QR code and saves response to qr_code_decode_response.
                qr_code_decode_response = system(system_command);



                // Creates a text file with the decoded QR code. The URL needed by the client is on the 5th line of the text file.
                FILE *decoded_qr_file;
                decoded_qr_file = fopen(decoded_qr_file_name, "r");
                char decoded_qr_line[256];
                int line_number = 0;
                while (fgets(decoded_qr_line, sizeof(decoded_qr_line), decoded_qr_file)) {
                    line_number++;
                    if (line_number == 5) {
                        strcpy(decoded_qr, decoded_qr_line);
                    }
                }
                fclose(decoded_qr_file);
                decoded_qr[strcspn(decoded_qr, "\n")] = 0;
                sprintf(log_message, "%s - %d - QR code decoded.", client_ip, client_id);
                write_to_log(log_message);
                sprintf(log_message, "%s - %d - Decoded QR code: %s", client_ip, client_id, decoded_qr);
                write_to_log(log_message);




                // Checking if decoded QR code is a valid URL.
                if (strstr(decoded_qr, "http://") != NULL || strstr(decoded_qr, "https://") != NULL) {
                    sprintf(log_message, "%s - %d - Decoded QR code is a valid URL.", client_ip, client_id);
                    write_to_log(log_message);
                } else {
                    sprintf(log_message, "%s - %d - Decoded QR code is not a valid URL.", client_ip, client_id);
                    write_to_log(log_message);
                    response_code = 1;
                }


                // Cleanup of all received files
                cleanup_decoded_qr(decoded_qr_file_name);
                sprintf(log_message, "%s - %d - Decoded QR code cleaned up.", client_ip, client_id);
                write_to_log(log_message);
                cleanup_qr_code(file_name);
                sprintf(log_message, "%s - %d - QR code cleaned up.", client_ip, client_id);
                write_to_log(log_message);
                

                // Sending response code and response message client.
                send(client_socket, &response_code, sizeof(int), 0);
                sprintf(log_message, "%s - %d - Response code sent to client.", client_ip, client_id);
                write_to_log(log_message);
                strcpy(server_message, "Success.");
                send(client_socket, &server_message, sizeof(server_message), 0);

                // Sending decoded QR code to client.
                send(client_socket, &decoded_qr, sizeof(decoded_qr), 0);

                // Logging end time of connection and closing file.
                close(client_socket);
                sprintf(log_message, "%s - %d - Client socket closed.", client_ip, client_id);
                write_to_log(log_message);
                printf("Handled client connection from %s, client ID %d.\n", client_ip, client_id);
                printf("Waiting for client connection...\n");
                exit(0);
            }

            // Client count decremented.
            client_count--;


        } else {


            // If maximum number of concurrent users has been reached, server closes client socket and waits for new connection.
            sprintf(log_message, "Maximum number of concurrent users reached.");
            write_to_log(log_message);
            printf("Maximum number of concurrent users reached.\n");
            close(client_socket);
            sprintf(log_message, "Client socket closed.");
            write_to_log(log_message);
            printf("Waiting for client connection...\n");

        }
    }
    
    return 0;

}
