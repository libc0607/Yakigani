# Yakigani
🚧 Under Construction 🚧  

### What's this? 
Unofficial Realtek RTL8370 iROM Burner  

### Why we need this?  
![8370-ds](https://github.com/libc0607/yakigani/blob/master/pic/8370.jpg)  
RTL8370 has a built-in DW-8051 kernel which will boot from the internal iROM.   
If you want to boot RTL8370 from spi nor flash, a small "bootloader" needs to be flashed into the internal iROM.  
However, Realtek does not sound like continuing to maintain the RTL8370 iROM programming tool   
(they use the parallel port to simulate I2C to program the chip, and it will run into errors when running on Win10 x64)   

### Where's the file to be programmed into this crab chip?  
Sorry, not now.

### How to compile?  
Use a STM32F103C8 Blue Pill  
Install Arduino  
Install Arduino_Core_STM32 for Arduino   
Install STM32CubeProgrammer   
Open the source code and read the comment    
