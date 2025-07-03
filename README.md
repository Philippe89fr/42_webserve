## Webserve - 42

# Description:
A high-performance HTTP web server implementation built from scratch in C++98. This project needs a deep understanding of network, HTTP protocol, and systems architecture by creating a fully functional web server capable of handling multiple clients, serving static content, processing CGI scripts, and managing file uploads. The server implements non-blocking I/O for optimal performance and scalability.

# Context:
Part of 42 Common Core.

# Main Technologies / Skills Used:

- **Programming Languages:** C++
- **Network Programming & Systems:** Socket programming, non-blocking I/O with select, and multi-client connection handling
- **HTTP Protocol Implementation:** HTTP/1.1 compliance, request/response parsing (status codes, headers management, and chunked transfer encoding)
- **C++98 Development:** Object-oriented design, Iterator/Containers, exception handling
- **Configuration Management:** Custom configuration file parsing, and route-based request handling
- **CGI Integration:** Common Gateway Interface implementation for dynamic content generation (eg. PHP, Python)
- **File System Operations:** Static file serving, directory listing, file upload handling, and secure path resolution
- **Concurrent Programming:** Event-driven architecture, file descriptor management, and process control for CGI execution

# Installation and compilation
1. Clone this repository into your project:
```bash
git clone git@github.com:Philippe89fr/42_webserve.git
```
2. Run the following command:
```bash
make 
```
3. To use the program, run the program and then enter any bash--posix usual command line:
```bash
./webserv conf_files/ConfigBase.conf
```
4. Open in your browser: http://localhost:4244/
