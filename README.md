# SensorNet Manager

## Overview

The **SensorNet Manager** is a C-based application designed to manage sensor data using a modular architecture. It provides an implementation for connecting to sensors, managing data, and storing it in a database. This project is organized into multiple components for better maintainability and scalability.

## Project Structure

The project consists of several modules, each responsible for a specific part of the system. Below is a brief description of the key components:

- **Makefile**: Used for compiling the project. It defines the compilation rules for building the executables and shared libraries.
- **config.h**: Header file containing configuration macros and constants used throughout the project.
- **connmgr**: Handles the connection management between the server and the sensors.
  - `connmgr.c` and `connmgr.h`: Implementation and interface for managing sensor connections.
- **datamgr**: Responsible for managing the sensor data received.
  - `datamgr.c` and `datamgr.h`: Implementation and interface for organizing and processing sensor data.
- **errmacros.h**: Header file defining macros for error handling throughout the project.
- **file_creator**: Handles file creation and management tasks.
  - `file_creator.c`: Implementation for creating files.
- **main**: The main application entry point.
  - `main.c` and `main.h`: Main application logic and definitions.
- **sbuffer**: Implements a shared buffer for storing data between components.
  - `sbuffer.c` and `sbuffer.h`: Implementation and interface for the shared buffer.
- **sensor_db**: Manages the interaction with the sensor database.
  - `sensor_db.c` and `sensor_db.h`: Implementation and interface for interacting with a SQLite database to store sensor data.
- **sensor_node**: Represents individual sensor nodes within the system.
  - `sensor_node.c`: Implementation for the sensor node.

### Libraries

The project also includes a set of libraries used for specific functionality:

- **lib/dplist**: A doubly linked list library for managing dynamic lists of data.
  - `dplist.c`, `dplist.h`, `libdplist.so`, `dplist.o`: Source, header, and compiled library files for the doubly linked list implementation.
- **lib/tcpsock**: A library for managing TCP socket connections.
  - `tcpsock.c`, `tcpsock.h`, `libtcpsock.so`, `tcpsock.o`: Source, header, and compiled library files for TCP socket operations.

## Compilation

To compile the project, use the provided **Makefile**. Simply run:

```sh
make
```
This will generate the necessary executables and shared libraries required for the project.

## Usage

After compiling the project, run the main executable to start the application. It will initiate connections to the sensor nodes and start managing the incoming data.

```sh
./main
```

## Dependencies

The project has the following dependencies:

- **SQLite3**: Required for managing sensor data in the database.
- **POSIX Threads**: Used for handling concurrency in the connection and data management modules.

Ensure that these dependencies are installed on your system before attempting to compile the project.

## Contributions

Feel free to contribute to the project by adding new features or improving existing functionality. Please follow standard C coding conventions and comment your code where necessary.

## License

This project is licensed under the MIT License. See the LICENSE file for more information.

## Contact

For any questions or issues, feel free to contact the project maintainers.

