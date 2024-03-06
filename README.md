# CS5008 Final Project
## John DeLeo
## Fall 2023

## Description
A simple server and client for sending a PNG image containing a QR code from a client, that is decoded and returned to client by the server.

## Supported platforms
- Tested on Ubuntu linux

## Dependencies
- A recent version of Java

## Package Files
- server.c
- client.c
- makefile
- README.md
- core.jar
- javase.jar
- project_qr_code.png (for testing purposes)

## Directions to compile
- cd to containing directory and run "make" to compile
- Two executables will be created:
    - server
    - client

    -- NOTE - running make will erase existing log files in the directory.

- To launch the server, run "./server". It can accept the following optional arguments:
    - PORT (Default - 2012)
    - RATE (Default - 2 60)
    - MAXUSERS (Default - 3)
    - TIMEOUT - (Default - 80)

    -- NOTE - If no argument is given, the default values above will be used.

    ### Example:
    ./server PORT 2000 RATE 5 60 MAXUSERS 5 TIMEOUT 60

    will attempt to launch the server on port 2000, set a rate limit of 5 attempts per user per 60 seconds, limit the number of simultaneous users to 5, and set a connection timeout to 60 seconds.

- To launch the client, run "./client". 
    - Required Argument:
        - QR followed by the path to a PNG file containing a QR code
    - Optional Arguments:
        - IP (of server) (Default - 127.0.0.1)
        - PORT (on server) (Default - 2012)

    -- NOTE - If no argument is given, the default values above will be used.

    ### Example:
    ./client QR 'project_qr_code.png' PORT 2000

    will attempt to connect to a server on the local computer (127.0.0.1) at port 2000. If successful, it will send "project_qr_code.png" to the server for decoding, and wait for a response.

    ./client QR 'project_qr_code.png' IP 192.168.1.200

    will attempt to connect to a server at '192.168.1.200' at the default port (Port 2012). If successful, it will send "project_qr_code.png" to the server for decoding, and wait for a response. 


Once launched, log files will be created in the same directory.
- server.log
- client.log
- access.log