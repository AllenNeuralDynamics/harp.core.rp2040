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
This is one of two entry points into writing a custom Harp-based application.
Like the HarpCore, the HarpCApp must also be a singleton.

## HarpApp
This the (Cpp) class-based entry point into writing a custom Harp-based application.
Like the HarpCore, the HarpApp must also be a singleton.

The HarpApp must be written by the user by deriving from the HarpCore and implementing the
`update_app_state`, `read_from_app_reg` and `write_to_app_reg` virtual functions.


## References
* [Pointer-to-Member Function Access](https://isocpp.org/wiki/faq/pointers-to-members#macro-for-ptr-to-memfn)
* [Array of Pointers to Member Functions](https://isocpp.org/wiki/faq/pointers-to-members#array-memfnptrs)
* [Private Virtuals](https://isocpp.org/wiki/faq/strange-inheritance#private-virtuals)
