## Compilation

```bash
make
```

Produces two binaries:
- `server`: file server
- `rfs`: client utility

Clean build files:
```bash
make clean
```

## Running

### Start Server
```bash
./server
```

### Run Client

#### WRITE
```bash
./rfs WRITE <local_file> <remote_path> <access> [-enc]
```
Example:
```bash
./rfs WRITE test.txt remote/test.txt RW
./rfs WRITE secret.txt remote/secret.txt RO -enc
```

#### GET
```bash
./rfs GET <remote_path> <local_file> [-enc]
```
Example:
```bash
./rfs GET remote/test.txt out.txt
./rfs GET remote/secret.txt out.txt -enc
```

#### RM
```bash
./rfs RM <remote_path>
```
Example:
```bash
./rfs RM remote/test.txt
```

## Encryption

- Enabled with `-enc` flag
- Client encrypts on WRITE, decrypts on GET
- Server stores data as-is

## Permissions

- `RW`: read-write
- `RO`: read-only (cannot be modified or deleted)
- Set only during file creation

## Example Workflow
```bash
./server
./rfs WRITE test.txt remote/test.txt RW -enc
./rfs GET remote/test.txt local.txt -enc
./rfs RM remote/test.txt
```

## Author
Prabesh Paudel and Vanessa Guan
CS5600 — Northeastern University
