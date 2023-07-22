 # Multi-User Chat Application README

This repository contains the source code for a multi-user chat application implemented in C++. The application enables users to communicate in real-time using a terminal-based interface. The server-client architecture is utilized to facilitate communication between multiple clients.

## Prerequisites

To run this application, you need the following installed on your system:

- C++ compiler
- pthread library

## Getting Started

1. Clone the repository to your local machine.

```bash
git clone https://github.com/ael-bekk/SimpelChat.git
cd SimpelChat
```

2. Compile the server and client files.

```bash
g++ -o server server.cpp -lpthread
g++ -o client client.cpp -lpthread
```

## Usage

### Server

To start the server, run the following command:

```bash
./server <port>
```

Replace `<port>` with the desired port number to listen for incoming connections.

### Client

To start a client, run the following command:

```bash
./client <server_IP> <port>
```

Replace `<server_IP>` with the IP address of the server and `<port>` with the same port number used for the server.

## How It Works

The multi-user chat application operates on a simple client-server model. The server listens for incoming connections from multiple clients, and each client can send and receive messages to and from others.

- The server sets up a listening socket on the specified port.
- Clients connect to the server using TCP sockets.
- Once connected, the clients can choose a username, and the server informs other clients about their entry.
- Clients can send messages to everyone in the chat or send private messages to specific users using the `@username` syntax.
- The server relays messages between clients, ensuring real-time communication.

## Features

- Real-time chat with multiple users.
- Private messaging using `@username` syntax.
- Terminal-based interface.
- User-friendly display of connected users.
- Support for ANSI color codes to distinguish messages.

## Contributors

This project was created by [ael-bekk](https://github.com/ael-bekk).

## License

This project is licensed under the MIT License - see the [LICENSE](https://github.com/ael-bekk/SimpelChat/blob/main/LICENSE) file for details.
