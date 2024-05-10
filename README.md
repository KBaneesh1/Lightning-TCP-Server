# Multithreaded TCP Server         

A multi-threaded server in C++, making use of PThreads for parallel programming.

## Requirements
 - g++
 - GNU Make (4.2.1 or higher)
 - [ncat (7.80 or higher)](https://nmap.org/ncat)
 - libgtkmm (4.0 or higher)

## Protocol Definition
The clients can open and work on a common file system. Multiple clients can read and modify files at the same time.

The server includes functions to
 - Optimize the request processing pipeline to handle multiple requests concurrently in parallel across the worker threads in the thread pool
 - Implement parallel content serving by allowing multiple worker threads to **read and write content concurrently** from the server
 - Implement a write-through **cache** for faster access of content
 - Implement parallel request routing and load balancing by allowing multiple worker threads to evaluate and route incoming requests concurrently


### All Commands

To run the server:
```bash
cd src/parallel/
make run_server
```

Run any number of clients with
```bash
cd src/parallel/
make run_client
```

# FAQ

1. Port already in use. This may happen if the server was not closed properly and the port is still in use.

    Solution: Kill the process using the port
    ```bash
    sudo lsof -i :8080
    kill -9 <PID>
    ```
2. `nc: HTTP/1.1 400 Bad Request Connection: close.` This may happen if the server is not running or netcat is pointed at the wrong port.
    
        Solution: Start the server and make sure the port is correct.
        ```bash
        ./bin/serial_server 8080
        ```
