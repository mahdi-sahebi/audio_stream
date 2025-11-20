# Client WebSocket

## Introduction
This project provides a **C++ client-side WebSocket implementation** for managing connections to a web server over HTTP and sending/receiving data streams.  
It is designed to be lightweight, modular, and easy to integrate into larger applications that require real-time communication.

---

## Tested Platforms

| Architecture | Operating System        | Status |
| :--:         | :--:                    | :--:   |
| x86          | Ubuntu 22.04.5 LTS      | PASSED |

---

## Dependencies

The project depends on **Google Test** for unit testing.

Install Google Test on Ubuntu:
```bash
sudo apt install libgtest-dev
```


## How to Build

Follow these steps to build the project:
```bash
# Create a build directory
mkdir build
cd build

# Run CMake
cmake ../

# Build the project
make

# Run the test binary
./test/test_audio_stream
```

<br>

## Tests

- Unit tests are implemented using Google Test.

- To enable tests, set the BUILD_TESTS option in CMake:
```bash
cmake -DBUILD_TESTS=ON ..
```

- After building, run the test suite:
```bash
./test/test_audio_stream
```

<br>
## Features

- WebSocket client connection: Establishes and manages a connection to a WebSocket server.
- Data transmission: Supports sending and receiving messages over the connection.
- Cross-platform build system: Uses CMake for portability.
- Unit testing: Integrated with Google Test for reliability.


<br>

## Future Work
- Add support for receive data
- Expand platform testing on Windows.
- Do the TODO tasks

