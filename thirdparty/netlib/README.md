## netlib
SDL_net library without the dependency on SDL

Adds byte buffer and utility methods:
- ```netlib_alloc_byte_buf(uint8_t size)```
    - free with ```netlib_free_byte_buf```
- write data with ```netlib_write_*```
    - supports 8, 16 and 32 bit signed/unsigned integers and floats
- read data with ```netlib_read_*```
- send with ```netlib_tcp_send_buf```
- receive with ```netlib_tcp_recv_buf```