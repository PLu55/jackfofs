# jack_fofs

## fof_queue

Fof\_queue is a circular array of chunks of fofs. Current sample index
divided by buffer size and masked by n\_slot, is used as pointer to the
current slot. It's fobidden to access the zeroth slot except for the
process manager.

In the queue the fofs are stored in chunks, the chunks belonging to a
slot are link together as a linked list.

n_slot is the number of slots, it must be a power of two.

## Testing

### Unity

https://www.throwtheswitch.org/unity/

Cheat Sheet:
https://github.com/ThrowTheSwitch/Unity/blob/master/docs/UnityAssertionsCheatSheetSuitableforPrintingandPossiblyFraming.pdf

### Unity and CMake

https://honeytreelabs.com/posts/cmake-unity-integration/

### Valgrind

`valgrind --leak-check=yes --track-origins=yes ./jfofstest`

### IPC

https://www.tutorialspoint.com/inter_process_communication/index.htm

https://opensource.com/sites/default/files/gated-content/inter-process_communication_in_linux.pdf

https://biendltb.github.io/tech/inter-process-communication-ipc-in-cpp/
