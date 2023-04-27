# jack_fofs

## fof_queue

Fof_queue is a circular array of chunks of fofs. Current sample index
divided by buffer size and masked by n_slot, is used as pointer to the
current slot. It's fobidden to access the zeroth slot except for the
process manager.

In the queue the fofs are stored in chunks, the chunks belonging to a
slot are link together as a linked list.

n_slot is the number of slots, it must be a power of two.
