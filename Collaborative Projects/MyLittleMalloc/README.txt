Authors: Tanay Kale(https://github.com/tvk134) & Anthony Paulino(https://github.com/antoutsider)

NOTE: The memory array is allocated with size "MEMSIZE + sizeof(metaData)" to account for the topmost chunk header that is always present.
If a specific limited size is desired, please set MEMSIZE to a value that is 4 less than desired.

0.) How to Compile
-------------------
to compile memgrind.c write to the terminal "make"
to compile correctness.c write to the terminal "make correctness"
to compile err.c write to the terminal "make err"


1.) Design Plan 
-------------------
Like noted previously, our memory array size is "MEMSIZE + sizeof(metaData)" to account for our start header that is initialized when malloc is called for the first time. 
Taking advatange that our global array is implicitly initialized to all zero, our malloc checks the first 4 bytes of our memory and checks whether a start header has been placed.
Our metadata is 4 bytes, so if everything still contains 0 in those first four bytes, then a header hasn't been placed, and malloc will place a header and assign the header's block size to MEMSIZE. (Since we return 4 bytes that our start header takes we assign size to MEMSIZE)
Malloc will then check for invalid sizes, which is 0 and anything less. As well as anything larger than our MEMSIZE. Malloc will return null and print an error message for invalid sizes. (Although our RequestChunk() function will not allocate an invalid size, it is more efficient to detect it before running our find block function)
If a valid size is requested then our requestChunk() method will iterate through the metadata header(s) and check their chunk sizes and see if there exist a chunk size that is large enough for the size that is being requested.
If a chunk size that large enough for the requested size does not exist it will return null and print an error message that there is insufficient memory available for the request. 
Otherwise if a chunk size large enough for the requested size exist, we will mark that chunksize as being used and return that chunksize to the user. 
We will check whether the header's chunk size is able to be split, that is if we subtract the header's chunksize with the amount that is requested, if what remains is large enough to hold a new metadata header and at least one byte, a new header will be created and assigned for that chunk minus the header size and be marked as available. What remains must be at least 5 bytes that is (4 bytes for header, 1 byte for chunk size since we aren't allowing 0 byte request)
Basically if what remains is large enough, we will assign the header's chunksize to size that was requested, and place a new header right after the requested chunk size that is going to be allocated/used and assign that new header's chunk size to the remaining bytes minus the header size (4 bytes). 
We are simply changing the header's chunk size to the requested size and what remains will be place a new header and the chunksize will be what remained minus the header size, we will proceed to return the chunk that was requested and not the new header created.
If the chunk size cannot be split up because the requested size is the same as the chunksize or because it is not large enough for a new header + 1 byte, we do not change the header's chunksize and just mark it as being used and return the address to that chunk.
Essentially every allocation will check if a new header can be created if not it doesn't, the client will always be returned a chunk large enough. 

When called, the free function first checks to make sure the provided pointer is not a NULL pointer and that it contains an address within the memory array. Both cases print relevant errors when failed.
Next, we create character and metaData pointers to the provided pointer address and the (presumed) memory chunk above it. The metaData pointer(called data) is passed to a function checkValid() that iterates through all of the valid chunk headers in memory starting from memory[0].
If pointer "data" is found to be equal to any of these, the function returns 1. Otherwise, it is clear that the pointer is either invalid or points to an address that is not the beginning of its chunk. A relevant error is printed.
Next, the "data" pointer's "valid" value is checked. If this value is 0, then the chunk in question has already been freed and a "Double free" error is returned. Otherwise, the chunk is now clear to be freed.
First, the metaData pointer's "valid" value is set to 0, marking it invalid and freed. After doing this, the coalesce() function is called as long as the freed chunk's value is not equal to MEMSIZE. In such a case, coalescing would not be necessary.
The coalesce function iterates through all available chunk headers in the memory array using the "chunkSize" value attributed to each. If a certain chunk is free and the chunk after it is also free, they are merged by adding the bottom chunk's chunkSize value along with the size of the metaData struct to that of the top chunk.
This ensures that any functions iterating over headers in the future will skip the header of the now freed chunk. The pointer to the next header is also reassigned to the next available. If the new next headr is also freed, it is also merged with the same top header.
The "current" or "top" header is not updated until the coalesce function comes across a chunk in memory directly below it that is valid. This ensures that when all of memory is freed, the final available chunk will return to chunkSize 4096 (or MEMSIZE) no matter what order the chunks are freed in.
Once the coalesce function reaches the end of the memory array (or what it recognizes to be the last available chunk header using simple pointer arithmetic), it is exited.

2.) Correctness Program 
-------------------
Please see "Correctness.txt" for information.

3.) Stress Test for 4.1 
-------------------
This test allocates, fills up, and then frees a 2-dimensional integer array and then a 3-dimensional integer array. All dimensions of both arrays are size 9.
It makes sure that the library can allocate, access, and free successive chunks of data in a reasonable amount of time. 

4.) Stress Test for 4.2 
-------------------
The goal for this test is to allocate a 1 byte chunk until there isn't suffiecient memory left to allocate. 
Memory size is 4096 + 4 bytes for our metadata start header that is initiliazed at the beginning. 
In other words, client's first allocation can be 4096. At most we can allocate 1 byte chunks 820 times. 
First allocation does not need bytes for a header since a starter header already exist, after the first allocation we will have 4095 bytes left and we would need 5 bytes per allocation for the new headers that are created after first allocation.
That is 1 + (4095/5) allocations, which is 820. That's is the reason for us creating an array pointer of 820 to store all of our pointers and free after. 
For freeing all of our 1 byte allocations, we freed even pointers first which will not require our coalesce function to merge the chunks, we then free our odd index pointers which will then require our coalesce function to start merging the chunks.