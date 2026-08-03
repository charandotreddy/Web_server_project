# High-Performance HTTP Server

A lightweight, bare-metal TCP/HTTP server written in C on Linux, built from scratch using native Unix socket APIs.

## Project Overview

### What is this?
This project is an event-driven web server built completely in C with zero external third-party framework dependencies. It interfaces directly with the Linux kernel to manage TCP connections, parse HTTP request streams, and serve static web assets back to clients.

### Why was it built?
Higher-level web frameworks abstract away the underlying mechanics of network transport and memory management. This project was created to gain a deep, foundational understanding of systems programming, socket communication, operating system resource management, and protocol handling at the file descriptor level.

## Features & Implementation

* Socket Lifecycle Management: Handles low-level socket creation, options configuration (SO_REUSEADDR), binding, listening, and accepting client connections.
* HTTP Request Parsing: Safely processes incoming raw network streams using fixed-size stack buffers to parse standard HTTP methods, paths, and versions without memory leaks.
* Static Asset Routing: Maps requested HTTP paths directly to local filesystem assets in a public directory structure.
* Standardized Protocol Responses: Generates fully compliant HTTP response headers, status codes (200 OK, 404 Not Found), content length indicators, and HTML bodies.
* Clean Shutdown Handling: Includes specialized routing mechanisms to handle graceful server termination and release active file descriptors.

## Project Structure

* src/ - Core C source files and network socket implementation.
* public/ - Static web page assets (index.html, about.html).
* bin/ - Compiled executable binaries (gitignored).
* docs/ - Low-level architectural notes, kernel observations, and phase logs.

## Build & Run Instructions

### Prerequisites
A standard C compiler (gcc) and Make utility installed on a Linux environment.

### Compilation
To compile the source code into a binary executable using standard safety flags (-Wall -Wextra -g):

```bash
make
