# `libblds-client`

## Overview

`libblds-client` is a library for interacting with the Baccus Lab
Data Server (BLDS) application over the network. It provides functionality
to communicate with the server; create, start and stop recordings;
query and maniuplate the managed source of data; and receive chunks
of data from a recording.

## Requirements and building

The following tools and libraries are required for using the C++
portion of the library:

- C++ compiler providing C++11 or later functionality
- [Qt 5.3](http://www.qt.io) or later
- [Armadillo](http://arma.sourceforge.net) C++ linear algebra library

To build, simply use:

 	$ qmake && make

You can build documentation using:

	$ doxygen

and test the library with:

	$ cd tests/
	$ qmake && make check

For the Python implementation, the requirements are:

- NumPy

## Usage

Please refer to the included documentation for details on the API.

