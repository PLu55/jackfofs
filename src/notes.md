# jack_fofs

## fof_queue

Fof\_queue is a circular array of slots where each slot holds a number
of chunks of fofs. A slot is one position in the circular
queue. Current sample index divided by buffer size and masked by
n\_slot, is used as pointer to the current slot. It's fobidden to
access the zeroth slot except for the process manager.

In the queue the fofs are stored in chunks, the chunks belonging to a
slot are link together as a linked list.

n_slot is the number of slots, it must be a power of two.

The writer adds fofs to the queue, a slot is selected base on the
activation time of the fof.

## Testing

### Unity

https://www.throwtheswitch.org/unity/

Cheat Sheet:
https://github.com/ThrowTheSwitch/Unity/blob/master/docs/UnityAssertionsCheatSheetSuitableforPrintingandPossiblyFraming.pdf

### Unity and CMake

https://honeytreelabs.com/posts/cmake-unity-integration/

### Valgrind

`valgrind --leak-check=yes --track-origins=yes ./jfofstest`

## IPC

https://www.tutorialspoint.com/inter_process_communication/index.htm

https://opensource.com/sites/default/files/gated-content/inter-process_communication_in_linux.pdf

https://biendltb.github.io/tech/inter-process-communication-ipc-in-cpp/

## Shared memory

### Memory mapping

A problem of shared memory is that pointer might not be handled
correcty by all participants as they may map the memory block to
different virtual address.

One strategy to handle pointers in shared memory is to try to use it's
first argument of mmap to map the adress to the same in all involved
processes. The'normal pointers can be used in the shared memory.
