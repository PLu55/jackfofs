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

## Lock free design

There are three types of jack clients, the ctrl client which handles
the fof_queue, the dsp client which handles the signal processing and
can run in parallel and the mix client which collects the signals from
the parallel dsp clients and frees the fof data. The dsp clients keep
it's own data which is handled by the fofs library.

The jack clients are connected and therby a computational order is
imposed, the ctrl client runs first then the dsp clients all in
parallel and last the mix client. With this construct it's not need to
lock any data. The only sycronisation needed is between the fof_queue,
the ctrl client and the api.

The api add fofs to the fof_queue which has a number of time slots
which slot the fof is added to depends on how far into the future the
start time of the fof is. If it's very close to the current time then
there is a need to syncronize the queue with the ctrl client,
otherwise no syncronization is needed.

Updating of the current time needs to be syncronized, the ctrl client
is updating the time of the fof_queue.

## Handling of fof data

Adding a fof to the fof_queue allocates memory from the shared memory.
When it's time to activate a fof the ctrl client add the fof to the
dsp client. The fof is then freed by the mix client. This is done to
facilitate the dsp clients to handle the fof data rather then ctrl
manager, this distributes the work of computing the fof parameters
to the mix client which are running in parallel.

## Testing

### Unity

<https://www.throwtheswitch.org/unity/>

Cheat Sheet:
<https://github.com/ThrowTheSwitch/Unity/blob/master/docs/UnityAssertionsCheatSheetSuitableforPrintingandPossiblyFraming.pdf>

### Unity and CMake

<https://honeytreelabs.com/posts/cmake-unity-integration/>

### Valgrind

`valgrind --leak-check=yes --track-origins=yes ./jfofstest`

## IPC

<https://www.tutorialspoint.com/inter_process_communication/index.htm>

<https://opensource.com/sites/default/files/gated-content/inter-process_communication_in_linux.pdf>

<https://biendltb.github.io/tech/inter-process-communication-ipc-in-cpp/>

## Shared memory

### Memory mapping

A problem of shared memory is that pointer might not be handled
correcty by all participants as they may map the memory block to
different virtual address.

One strategy to handle pointers in shared memory is to try to use it's
first argument of mmap to map the adress to the same in all involved
processes. The'normal pointers can be used in the shared memory.

## Running under PipeWire

Delay of the effect of jack commands take longer then running directly
under jack. The consequences are that delays has to be entered in
testing procedures.
