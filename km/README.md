# Assignment 3: Named Pipe for Exchanging Numbers
A kernel module `numpipe` that implements a UNIX named pipe (FIFO queue) as a character device. User-level producers and consumers can add and remove integers to the shared FIFO queue, respectively.

## Build numpipe
Create a character device file associated with `numpipe`:
```
mknod /dev/numpipe c <Major> <Minor>
```

Build kernel module:
```
cd numpipe
make
```

Build producer and consumer executables. If currently in the directory `numpipe/`, go back a directory:
```
cd ../
make
```

## Loading and running module
Load the kernel module. Can specify the maximum size N of the shared FIFO queue (default size is 20):
```
cd numpipe
sudo insmod numpipe.ko max_size=N
```

Run a producer. If currently in the directory `numpipe/`, go back a directory:
```
cd ../
./producer
```

Run a consumer. If currently in the directory `numpipe/`, go back a directory:
```
cd ../
./consumer
```

Remove a producer/consumer with Control+C.
Can run multiple producers and consumers in separate terminal windows. Each should block if the desired conditions are not true (queue is full or empty).
