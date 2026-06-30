# 📝 HTTP Server Development Notes

## 📌 Phase 1: Socket Skeleton & Connection Mechanics

### 🧱 Architectural Breakdown (The 3 Pillars)
For each system call introduced in this phase, here is how the API, the operating system kernel, and potential traps map out:

#### 1. `socket()`
* **Signature:** `int socket(int domain, int type, int protocol);`
* **Kernel Action:** Allocates a raw entry in the process's file descriptor table, initializing a foundational network context resource.
* **Gotchas:** Returns `-1` on failure; must always be tracked and handled to prevent silent resource allocation errors.

#### 2. `setsockopt()`
* **Signature:** `int setsockopt(int sockfd, int level, int optname, const void *optval, socklen_t optlen);`
* **Kernel Action:** Reconfigures socket rules at the `SOL_SOCKET` level. Activating `SO_REUSEADDR` permits bypassing the kernel's defensive `TIME_WAIT` cooldown lock.
* **Gotchas:** Must be invoked *after* socket creation but strictly *before* executing `bind()`, otherwise the kernel ignores the modification.

#### 3. `bind()`
* **Signature:** `int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);`
* **Kernel Action:** Maps the abstract socket file descriptor to a physical network interface configuration (IP and Port) using the `struct sockaddr_in` structure.
* **Gotchas:** Passing a local port address without applying network byte order formatting (`htons()`) causes network routing lookup to fail.

#### 4. `listen()`
* **Signature:** `int listen(int sockfd, int backlog);`
* **Kernel Action:** Transforms an active socket into a passive connection receiver, establishing a double-queue system (`SYN_RCVD` and fully established connections) bound to the maximum size defined by the `backlog`.
* **Gotchas:** It does not pause or block process execution flow; it runs immediately and returns control to the next line of code.

#### 5. `accept()`
* **Signature:** `int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);`
* **Kernel Action:** A blocking system call that pauses the process until a connection request arrives in the queue. Upon a successful handshake, it pops the client from the queue and returns a brand-new, distinct file descriptor dedicated entirely to that specific client session.
* **Gotchas:** Modifies client tracking variables in-place, meaning pointers (`&`) to the address struct and size variable must be explicitly passed.

---

### 💻 Execution & Verification Notes

#### File Descriptor Observations
* **Listening Socket Descriptor (`server_fd`):** Assigned as `3`. This serves exclusively as the main entry point to hear incoming connection requests.
* **Client Connection Descriptor (`client_fd`):** Assigned as `4`. This is the dedicated communication socket generated specifically to read and write bytes with the active client.

#### Port & Connection Details
* **Server Listening Port:** `8080`
* **Loopback IP:** `127.0.0.1` (localhost)
* **Client Ephemeral Port Mapping:** Observed a random high-numbered port allocation assigned dynamically by the client OS to manage the outgoing side of the TCP handshake pipe.

---

### 🔍 Debugging & System Behavior Discoveries

* **Address Already in Use Error:** Attempting to execute a secondary concurrent instance of the server binary against port `8080` throws a `bind() failed: Address already in use` error. This demonstrates the OS kernel's strict restriction against mapping multiple distinct listener endpoints to the exact same transport address profile simultaneously.
* **Memory and Resource Lifecycle:** Confirmed that invoking `close()` systematically against both active file descriptors cleanly removes their table indices from the process descriptor register, yielding safe program termination.

----------------------------------phase 2--------------------------
##phase 2 :Reading raw data and network streams
Here is the exact network text stream captured from the `curl` client:
```text
GET / HTTP/1.1
Host: 127.0.0.1:8080
User-Agent: curl/8.5.0
Accept: */*

-----------------------
system call choice read() between recv()
    selected recv() with a trailling parameter of 0.
    while read() works because sockets are tracked as standard file descriptors  in Linux.
    ---
    recv() is network explicit.
    it signals a dedicated  socket connection state context  and provides access to socket specific behaviour modifiers.
Edge case analysis
    setting buffer[1024] limit to 1024 bytes provide a safe memory constraint frame.
    if it is more than 1024 then it would read 1023 bytes and processes it as a complete string.


    ----------------------------------------------------
Phase 3: Reading the Request & Sending a Real Web Page
    ----------------------------------------------------

1. What does an HTTP Response look like?
When a web browser asks for a page, your server must reply with a highly structured block of text. It is always broken down into 4 distinct parts:

The Status Line: Tells the browser that everything worked fine (HTTP/1.1 200 OK).

The Headers: Metadata lines that describe the content. We use Content-Type: text/html to tell the browser it is receiving a web page, and Content-Length to say exactly how big the page is.

The Blank Line (\r\n): A mandatory empty line that acts as a wall. It tells the browser, "The metadata is done! The actual visual website starts right after this."

The Body: The actual HTML code (<html><body>...</body></html>) that gets drawn on the screen.

------------------------------------------------------------------

2. Why we chose sscanf for parsing
We used sscanf(buffer, "%s %s %s", method, path, version) because it is safe and incredibly clean. It looks at the very first line of the incoming data and extracts the words separated by spaces. It does this without destroying or modifying our original data buffer.
-----
******************************************************8
-----

Our Experimental Observations
1. The /foo Path Test
What we did: We opened a browser and typed [http://127.0.0.1:8080/foo](http://127.0.0.1:8080/foo).

What happened: The browser still displayed the big, bold HELLO page, but our terminal successfully printed out Path: /foo.

What this proves: This proves our server can read custom paths perfectly without crashing! It still shows the same page because our response code is currently hardcoded to send back the same HTML string no matter what.

----------------

2. Fixing the Header Bug
What we did: We fixed our headers to use a standard hyphen (Content-Type instead of content_type) and removed the extra space before the colon.

What happened: Automated tools like curl stopped complaining. Instead of shutting down the connection with an error or warning, curl cleanly reported: Connection left intact.

What this proves: Writing code that perfectly follows global standards makes sure that every single web browser can talk to your server without unexpected glitches.

----------------

3. The Content-Length Experiment
What happens if you remove it? If you temporarily delete Content-Length, the browser's loading spinner will spin endlessly, and curl will hang in the terminal without exiting.

Why does it break? Because modern web connections stay open to send multiple files, the client has no idea when your server is done talking. Without a Content-Length telling it exactly how many bytes to look for, it will sit there waiting forever.

