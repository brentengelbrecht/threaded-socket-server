![GitHub repo size](https://img.shields.io/github/repo-size/brentengelbrecht/threaded-socket-server)
![GitHub contributors](https://img.shields.io/github/contributors/brentengelbrecht/threaded-socket-server)
![GitHub stars](https://img.shields.io/github/stars/brentengelbrecht/threaded-socket-server?style=social)
![GitHub forks](https://img.shields.io/github/forks/brentengelbrecht/threaded-socket-server?style=social)

# Threaded socket server

Compact server to accept new socket connections and run them in a separate thread.

- Dynamically-sized thread count -> adjust while the app is running
- Add pending connections to a queue while waiting for a service thread
- Timeout sockets that can't be serviced within a specific time

## To build (Linux)

Run this command on command line: `gcc -o server server.c queue.c key_list.c`
