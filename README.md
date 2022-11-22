# Threaded socket server

Compact server to accept new socket connections and run them in a separate thread.

- Uses a dynamically-size thread count
- Add connections waiting for a thread to a queue for servicing
- Timeout sockets that can't be serviced within a specific time

## To build (Linux)

Run this command `gcc -o server server.c queue.c key_list.c`
