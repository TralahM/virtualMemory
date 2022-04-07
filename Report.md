# Report
## Implementation of Algorithms

Data structure Used **Hash Table** to keep count of the number of occurrences of
page addresses.

The FIFO algorithm maintains a list of length **nframes** as specified in the
command line argument parsed. An index variable is also maintained to keep track
of the oldest entry in the page frames.

The LRU algorithm maintains a hash table counting the number of occurrences and
keeps also a list of length **nframes**.

FIFO shows an increase in page faults as the number of frames increases.

Belady's anomaly is observed in FIFO implementation of the algorithm.

**LRU** performs on average better with fewer reads than the **FIFO** implementation.

Example: with the gcc file
```
./virtual_memory gcc.txt 3 fifo
Contents of page frames
41f7a0  13f5e2c0        3d729358
Number of Reads: 772131
Number of Writes: 104093
```

and

```
./virtual_memory gcc.txt 3 lru
Contents of page frames
3d729358  4648e0    464cc8
Number of Reads: 772029
Number of Writes: 104087
```
