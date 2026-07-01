# HTTP Server Development Notes

## Phase 1: Socket Skeleton & Connection Mechanics

**Phase Description:** This phase establishes the foundational network socket, binds it to a local port, and configures the kernel to listen for and accept incoming client connections.

### 1. Architectural Breakdown

* **`socket()`**
Allocates a raw entry in the process's file descriptor table to initialize an IPv4 TCP network context. It returns `-1` on failure, which must be handled to avoid running on an unallocated resource.
* **`setsockopt()`**
Configures the socket with `SO_REUSEADDR` before binding to bypass the kernel's `TIME_WAIT` cooldown lock. This allows the server to restart instantly on the same port without waiting for old connections to clear.
* **`bind()`**
Associates the abstract socket file descriptor with a physical local IP and port number using a `struct sockaddr_in`. The port number must use `htons()` to convert it to network byte order or routing will fail.
* **`listen()`**
Converts the active socket into a passive receiver and sets up background queues to hold pending connection handshakes. It returns control to the program immediately without halting execution flow.
* **`accept()`**
A blocking call that pauses the server until a client connects, then pops that client from the queue. It updates client tracking variables via pointers and returns a new file descriptor used exclusively for that session.

### 2. Key Observations & System Behavior

* **File Descriptors:** The listening socket (`server_fd`) sits at descriptor `3` to receive incoming traffic, while each connected client gets a dedicated descriptor (`client_fd`) starting at `4` for private read/write operations.
* **Port Restrictions:** Running a duplicate instance of the server on port 8080 throws an "Address already in use" error during the `bind()` step because the kernel prevents multiple processes from sharing a transport profile.
* **Resource Cleanup:** Invoking `close()` on active file descriptors cleanly deletes them from the system table, preventing memory and resource leaks upon program termination.

---

## Phase 2 & 3: Request Reading & Page Parsing

**Phase Description:** This phase reads raw HTTP request streams from the network, extracts routing paths, and transmits properly formatted HTML responses back to the client.

### 1. Reading & Parsing Streams

* **`recv()` vs `read()**`
We use `recv()` with a flag of `0` because it explicitly indicates network-specific socket operations rather than generic file I/O. The input buffer is locked at 1024 bytes to create a safe memory boundary and prevent data spilling.
* **`sscanf()` Strategy**
The request line is processed using `sscanf(buffer, "%s %s %s", method, path, version)`. This extracts the space-separated HTTP tokens directly from the first line without editing or corrupting the raw incoming data buffer.

### 2. HTTP Response Structure

* **Status Line:** Sent first to notify the browser of the transaction result (such as `HTTP/1.1 200 OK` or `404 Not Found`).
* **Headers:** Metadata lines describing the payload, specifically using `Content-Type: text/html` for page rendering and `Content-Length` to state the exact payload size.
* **Blank Line (`\r\n`):** A mandatory empty line acting as a structural divider that tells the client the metadata headers are finished and the payload body follows.
* **Body:** The raw HTML code transmitted immediately after the blank line that the browser processes and displays on screen.

### 3. Experimental Discoveries

* **Path Testing:** Accessing a custom path like `/foo` updates the server log to read `Path: /foo` but still serves the default page because the initial response logic uses a hardcoded HTML string.
* **Standardization:** Correcting header syntax to use a standard hyphen (`Content-Type`) prevents network utilities like `curl` from throwing protocol errors or abruptly terminating connections.
* **Content Length:** Removing the `Content-Length` header causes web browsers to spin indefinitely and `curl` to hang because the client cannot determine when the transmission ends over a persistent TCP connection.
