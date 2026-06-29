# 🚀 High-Performance HTTP Server

A lightweight, bare-metal TCP/HTTP server written in C on Linux (Ubuntu 24.04), built completely from scratch using the native socket APIs.

## 📁 Project Structure
* `src/` - Core C source files.
* `bin/` - Compiled machine binaries (Gitignored).
* `docs/` - Running architectural notes and phase summaries.

## 🛠️ Build & Run Instructions

### Prerequisites
Make sure you have a standard C compiler (`gcc`) and `make` installed.

### Compilation
To compile the server using the optimized compilation flags (`-Wall -Wextra -g`), run:
```bash
make
