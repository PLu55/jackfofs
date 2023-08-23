# JackFofs bug list

* ~~Number of fofs in the initial free list can not be set in main,
  error will occure frequently.~~
* when server gets overloaded there is problems with memory allocation
  malloc(): invalid size (unsorted) and it uses cpu when no fofs suppose
  to be active.
* better structure of sources and headers 
* check test_api
* check handling of setup data
