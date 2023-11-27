# C Language Vector Library

## Overview
This repository contains an implementation of a dynamic array (vector) library in C. The `Vector` struct provides a flexible and generic container for managing a resizable array of elements.

## Features
- Dynamic resizing
- Generic element storage
- Basic operations: insertion, deletion, access, and modification of elements
- Utility functions: sorting, reversing, finding
- Safe memory allocation and reallocation

## Getting Started

### Prerequisites
- A C compiler (e.g., GCC)
- Basic understanding of C programming

### Installation
Clone this repository or download the source files into your project directory.


Include `vector.h` and `vector.c` in your project.

### Usage
Here's a basic example of how to use the `Vector` library:

```c
#include "vector.h"

int main() {
    // Create a new vector of integers with initial capacity of 10
    Vector myVector = createVector(10, sizeof(int));

    // Add an element to the vector
    int element;
    pushBack(&myVector, addInt(&element, 5);
    pushBack(&myVector, addInt(&element, 10);

    // Access and modify elements
    int *elemPtr = (int *)get(&myVector, 0);
    if (elemPtr != NULL) {
        *elemPtr = 10;
    }

    // Clean up
    deleteVector(&myVector);

    return 0;
}


