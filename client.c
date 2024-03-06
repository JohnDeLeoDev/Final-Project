// client.c is a client program that will connect to the server

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h> 
#include <time.h>
#include <netinet/in.h>
#include <signal.h>


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
    log_file = fopen("./client.log", "a");
    fprintf(log_file, "%d-%02d-%02d %02d:%02d:%02d - %s\n", current_year, current_month, current_day, current_hour, current_minute, current_second, message);
    fclose(log_file);
}


// function to determine qr code size
int get_qr_code_size(char *qr_code) {
    FILE *qr_file;
    qr_file = fopen(qr_code, "r");

    // get file size
    fseek(qr_file, 0, SEEK_END);
    int file_size = ftell(qr_file);
    fseek(qr_file, 0, SEEK_SET);

    fclose(qr_file);

    return file_size;
}

// function to read qr code from file
char *read_qr_code(char *qr_code) {
    FILE *qr_file;
    qr_file = fopen(qr_code, "r");

    // get file size
    fseek(qr_file, 0, SEEK_END);
    int file_size = ftell(qr_file);
    fseek(qr_file, 0, SEEK_SET);

    // read qr code
    char *qr_code_data = (char *)malloc(file_size + 1);
    fread(qr_code_data, 1, file_size, qr_file);
    qr_code_data[file_size] = '\0'; // Null-terminate the string
    fclose(qr_file);

    return qr_code_data;
}

int main(int argc, char *argv[]) {
    int client_socket;
    int port = 2012;
    char server_response[256];
    char client_message[256];
    char log_message[512];
    char *server_ip = "127.0.0.1";
    char *qr_code = NULL;
    int timeout = 500;

    void handle_exit(int sig) {
        close(client_socket);
        sprintf(log_message, "Client exited.");
        write_to_log(log_message);
        sprintf(log_message, "##############################################");
        write_to_log(log_message);
        printf("\nClient exited.\n");
        exit(0);
    }

    signal(SIGINT, handle_exit);

    void timeout_handler(int signum) {
        sprintf(log_message, "Timeout after %d seconds.", timeout);
        write_to_log(log_message);
        printf("%s\n", log_message);
        sprintf(log_message, "##############################################");
        write_to_log(log_message);
        exit(0);
    }

    // set timeout
    signal(SIGALRM, timeout_handler);
    alarm(timeout);

    sprintf(log_message, "##############################################");
    write_to_log(log_message);


    if (argc < 2) {
        sprintf(log_message, "Client started with no arguments passed. Exiting.");
        write_to_log(log_message);
        sprintf(log_message, "No arguments passed.\nTo launch client, follow example below.\n\nExample:\n./client QR 'project_qr_code.png'\n\nExiting.");
        printf("%s\n", log_message);
        sprintf(log_message, "##############################################");
        write_to_log(log_message);
        
        exit(0);
    }

    sprintf(log_message, "Client starting with arguments: ");
    for (int i = 0; i < argc; i++) {
        strcat(log_message, argv[i]);
        strcat(log_message, " ");
    }
    write_to_log(log_message);

    // access qr code in argument
    if (argc > 1) {
        for (int i = 1; i < argc; i++) {
            if (strcmp(argv[i], "QR") == 0) {
                qr_code = argv[i + 1];
            }
        }
    }

    // checks if qr code provided by user
    if (qr_code == NULL) {
        sprintf(log_message, "No QR code provided.");
        write_to_log(log_message);
        sprintf(log_message, "No QR code provided.\nTo launch client, follow example below.\n\nExample:\n./client QR 'project_qr_code.png'\n\nExiting.");
        printf("%s\n", log_message);
        sprintf(log_message, "##############################################");
        write_to_log(log_message);
        exit(0);
    }

    // checks if qr file exists
    FILE *qr_file;
    qr_file = fopen(qr_code, "r");
    if (qr_file == NULL) {
        sprintf(log_message, "QR code file does not exist. Exiting.");
        write_to_log(log_message);
        printf("%s\n", log_message);
        sprintf(log_message, "##############################################");
        write_to_log(log_message);
        exit(0);
    } 
    fclose(qr_file);

    sprintf(log_message, "QR code: %s", qr_code);
    write_to_log(log_message);

    // access server ip in argument
    if (argc > 1) {
        for (int i = 1; i < argc; i++) {
            if (strcmp(argv[i], "IP") == 0) {
                server_ip = argv[i + 1];
            }
        }
    }
    sprintf(log_message, "Server IP: %s", server_ip);
    write_to_log(log_message);

    // access port in argument
    if (argc > 1) {
        for (int i = 1; i < argc; i++) {
            if (strcmp(argv[i], "PORT") == 0) {
                port = atoi(argv[i + 1]);
            }
        }
    }
    sprintf(log_message, "Port: %d", port);
    write_to_log(log_message);

    // create socket
    client_socket = socket(AF_INET, SOCK_STREAM, 0);
    sprintf(log_message, "Client socket created.");
    write_to_log(log_message);

    // specify an address for the socket
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port);
    server_address.sin_addr.s_addr = inet_addr(server_ip);
    sprintf(log_message, "Server address specified.");
    write_to_log(log_message);

    // connect to server
    int connection_status = connect(client_socket, (struct sockaddr *) &server_address, sizeof(server_address));

    if (connection_status == -1) {
        sprintf(log_message, "There was an error making a connection to the remote socket.");
        write_to_log(log_message);
        printf("%s\n", log_message);
        close(client_socket);
        sprintf(log_message, "Client socket closed.");
        write_to_log(log_message);
        sprintf(log_message, "Client exited");
        write_to_log(log_message);
        sprintf(log_message, "##############################################");
        write_to_log(log_message);
        printf("\nClient exited.\n");
        exit(0);
    } else {
        sprintf(log_message, "Connection successful.");
        write_to_log(log_message);
    }

    // receive response code from server
    int connection_response_code;
    recv(client_socket, &connection_response_code, sizeof(int), 0);
    sprintf(log_message, "Response code received from server: %d", connection_response_code);
    write_to_log(log_message);
    printf("Connection response code: %d\n", connection_response_code);

    if (connection_response_code != 0) {
        recv(client_socket, server_response, sizeof(server_response), 0);
        sprintf(log_message, "Response message received from server: %s", server_response);
        write_to_log(log_message);
        printf("%s\n", log_message);
        sprintf(log_message, "##############################################");
        write_to_log(log_message);
        exit(0);
    }

    // receive data from the server
    recv(client_socket, server_response, sizeof(server_response), 0);
    sprintf(log_message, "Server response: %s", server_response);
    write_to_log(log_message);

    // send qr code size to server
    int outgoing;
    int qr_code_size = get_qr_code_size(qr_code);
    outgoing = write(client_socket, &qr_code_size, sizeof(int));
    if (outgoing < 0) {
        sprintf(log_message, "Error sending qr code size to server.");
        write_to_log(log_message);
        exit(0);
    }
    sprintf(log_message, "QR code size sent to server.");
    write_to_log(log_message);
    printf("Sending QR code to server...\n");


    // send qr code to server
    char *qr_code_data = read_qr_code(qr_code);
    outgoing = send(client_socket, qr_code_data, qr_code_size, 0);
    if (outgoing < 0) {
        sprintf(log_message, "Error sending qr code to server.");
        write_to_log(log_message);
        exit(0);
    }
    sprintf(log_message, "QR code sent to server.");
    write_to_log(log_message);
    printf("QR code sent to server.\n");
  
    // receive response code from server
    int response_code;
    recv(client_socket, &response_code, sizeof(int), 0);
    sprintf(log_message, "Response code received from server: %d", response_code);
    printf("Response code: %d\n", response_code);
    write_to_log(log_message);


    // receive 1st response message from server
    char response_message[256];
    recv(client_socket, response_message, sizeof(response_message), 0);
    sprintf(log_message, "Response message received from server: %s", response_message);
    printf("Response message: %s\n", response_message);

    if (response_code == 1) {
        write_to_log(log_message);
        sprintf(log_message, "##############################################");
        write_to_log(log_message);
        exit(0);
    }

    // receive decoded qr code from server
    char decoded_qr_code[256];
    recv(client_socket, decoded_qr_code, sizeof(decoded_qr_code), 0);
    sprintf(log_message, "Decoded qr code received from server: %s", decoded_qr_code);
    write_to_log(log_message);
    printf("%s\n", log_message);
  
    // close the socket
    close(client_socket);
    sprintf(log_message, "Client socket closed.");
    write_to_log(log_message);
    sprintf(log_message, "##############################################");
    write_to_log(log_message);

    return 0;
 
}
   