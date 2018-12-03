# Assignment 3: Named Pipe for Exchanging Numbers
A kernel module `numpipe` that implements a UNIX named pipe (FIFO queue) as a character device. User-level producers and consumers can add and remove integers to the shared FIFO queue, respectively.

## Build numpipe
1. Build kernel module:
```
cd numpipe
make
```
2. Set permissions on `numpipe` device file:
```
sudo chmod 666 /dev/numpipe
```

3. Build producer and consumer executables. If currently in the directory `numpipe/`, go back a directory:
```
cd ../
make
```

## Loading kernel module
Load the kernel module, if the module has not been loaded (can check with `lsmod`). Can specify the maximum size N of the shared FIFO queue (default size is 20):
```
cd numpipe
sudo insmod numpipe.ko max_size=N
```

## Using `numpipe` with producers and consumers

2. Run a producer. If currently in the directory `numpipe/`, go back a directory:
```
cd ../
./producer /dev/numpipe
```

3. Run a consumer. If currently in the directory `numpipe/`, go back a directory:
```
cd ../
./consumer /dev/numpipe
```

Remove a producer/consumer with Control+C.
Can run multiple producers and consumers in separate terminal windows. Each should block if the desired conditions are not true (queue is full or empty).
