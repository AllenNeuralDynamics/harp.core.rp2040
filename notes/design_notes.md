# Design Notes

## Harp Core
### Overview
The Harp Core
* polls the usb serial port for incoming messages
* parses received messages into their respective fields
* dispatches READ and WRITE messages to their respective core register handler functions
* provides a means of being subclassed such that "Harp Apps" can be built and extended. Specifically:
  * provides a virtual `update_app_state` that a derived class can implement.
  * provides virtual app read and write functions that a derived class can implement.

### Update Function
Derived classes with custom update behavior must override the virtual member function `update_app_state` to handle app-specific update behavior from within the `run()` function.

## Harp C App
This is the main entrypoint for writing a custom Harp app.

Harp protocol involves the PC reading and writing to specific, fixed locations in the devices memory referred to as *registers*.
Reading or writing to a register will cause the device to issue a reply from that register and can additionally trigger device-specific actions.
Harp devices can send event messages at any time without requests from the PC.

In defining a custom Harp App, you get to decide (1) what registers exist, (2) what data they contain, and (3) what additional actions happen when the PC reads-or-writes to those registers.

### The App Recipe
Simply:
* Create a struct of elements to serve as your device's Harp registers.
* Create a struct of `RegSpecs` to enable fast iteration through the struct. (Currently, this is a bit redundant and cannot be inferred easily from the code.)
* Create a struct of read/write handler functions, one per register.
* Define an `update` function for the app
* Define a `reset` function for the app
* instantiate the `HarpCApp` class
* In a loop, periodically call the `update()` function (we suggest a frequency of 2KHz).

To see this design pattern in an example, check out the examples folder.


## References
* [Pointer-to-Member Function Access](https://isocpp.org/wiki/faq/pointers-to-members#macro-for-ptr-to-memfn)
* [Array of Pointers to Member Functions](https://isocpp.org/wiki/faq/pointers-to-members#array-memfnptrs)
* [Private Virtuals](https://isocpp.org/wiki/faq/strange-inheritance#private-virtuals)
