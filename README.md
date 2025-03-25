# cs5600-practicum


## Compilation and Execution

To compile and run the program, use the following commands:

### 1. Clean previous build files (optional but recommended)
```sh
make clean
```

### 2. Compile the project
```sh
make
```

### 3. Run the program with different replacement algorithms:
- **Random Replacement**:
  ```sh
  ./main Random
  ```
- **Least Recently Used (LRU)**:
  ```sh
  ./main LRU
  ```
- **First In First Out (FIFO)** (default if no matching policy is specified):
  ```sh
  ./main {anything}
  ```
