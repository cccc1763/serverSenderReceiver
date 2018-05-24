============================================================
    server-sender-receiver
============================================================
 gcc server.c -o server -lpthread
 gcc sender.c -o server -lpthread
 gcc receiver.c -o server -lpthread
============================================================
 usage ./server <port>
 usage ./sender <ip> <port> <nickname> <group>
 usage ./receiver <ip> <port> <nickname> <group>
============================================================