# DrawOnMe
Program for drawing to a Raspberry Pi.
## Running
Compile:

    make drawonme

I recommend running this from the CLI rather than X.

Running:

    ./drawonme

Program can be killed by Ctrl + C. After stopping the program, the display will probably be in a strange state. Either reboot or use something like `fbset -g 640 400 640 400 8` to make things better. Listens on UDP port 8888. 
## Specificiation
### About
32,000 bytes of video RAM are available for drawing to the screen. The screen is 320×200 pixels, supporting a palette of 16 colors. See https://en.wikipedia.org/wiki/Color_Graphics_Adapter#Color_palette for the color palette. Each byte of video memory holds two pixels. The high 4 bits contains the left pixel and the low 4 bits contains the right pixel. Address 0 represents the upper left corner of the screen and progresses left to right, top to bottom across the screen.
### Protocol
Packets are sent via UDP to 172.16.5.62 port 8888. Maximum data payload is 4096 bytes. There are two types of data: set address and set value. An individual packet can contain multiple values as long as the overall payload is less than 4096 bytes.
#### Set Address 
This packet sets the address register. It consists of two bytes. The most-significant bit of the first byte is 0 and the remaining bits represent the address. Behavior of addresses above 32,000 is undefined.

Binary: `0aaa aaaa  aaaa aaaa`
#### Set Value 
This packet sets the video RAM at the address in the address register. Each byte that is written to memory automatically increases the address register by one. The most-significant bit of the first byte is 1 and the remaining bits represent the number of bytes (0 - 127) to be written to video RAM. 

Binary: `1ccc cccc`

Example of writing a single byte (blue pixel followed by light green pixel) to the current memory address:

Binary: `1000 0001  0001 1010`
### Miscellaneous
Example payload of drawing a light red face in the upper left, represented in hexadecimal:

    0000 840c cccc c000 a084 cccc cccc 0140
    84cc 0cc0 cc01 e084 cccc cccc 0280 84cc
    0000 cc03 2084 ccc0 0ccc 03c0 84cc cccc
    cc04 6084 0ccc ccc0
