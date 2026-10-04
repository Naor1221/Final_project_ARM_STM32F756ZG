ARM STM32F756ZG testing program overview: 
This program receives structure, which was sent from PC via UDP protocol through usb cabal.
The structure will include: ID, priphery or pripheries to be checked, message and message length.
According to the bits that are ON in priphery or pripheries to be checked, test will be made: 
bit 0 - Timer test
bit 1 - UART test
bit 2 - SPI test
bit 3 - I2C test
bit 4 - ADC test 
Explantion for each test: 
Timer test: 
Checks if advance timer (timer1) counts to the same value, as general purpose(timer4) does.
Both timers use output compare mode, and have the same ARR.
One pulse mode is enabled,making sure every iteration each timer counts once.
If one of the timers dont finish it's counting within 0.5 seconds, test result is fail.
If both timers reach the same ARR as excepted, in less than 0.5 seconds, test result is success. 
**timer test uses interrupts. 

UART,SPI and I2C:
All three have the same test,UART and SPI use DMA but I2C uses interrupts. 
for UART the test was done with uart5 and uart7 ports.
for SPI the test was done with spi1(master) and spi4(slave) ports. 
for I2C the test was done with i2c1 and i2c4 ports.
Test will be done as following: 
First, the message that was received from the incomming struct is transferd to one port(port0), 
then this port sends the message to the other port(port1). 
After this, the other port(port1) will send back the message to the first port(port0).
Finally, the incomming message to the first port(port0) will be compared to the message,coming from the incomming struct.
if message len is above 100 chars, CRC check will take place instead. 
Test result is considered as success only if the compare is true. 

ADC test: 
Using bulit in temprature inside STM32F756ZG, an analog input is converted into digital value
if the value is between 25 to 50 celcius degrees, test result is considered as success. 
**finding temperature value was done with the formula in reference manual page 441,item 8. 
**ADC test uses DMA,and ADC1.

Hardware requirements: 
STM32F756ZG
USB to USB cable
GPIO pins: 
    UART:
      uart5 tx -PC12
      uart5 rx -PD2
    
      uart7 rx -PE7
      uart7 tx -PE8
    
    SPI:
      spi1 sck -PA5
      spi1 miso -PA6
      spi1 mosi -PB5
    
      spi4 sck -PE2
      spi4 miso -PE5
      spi4 mosi -PE6
    
    I2C:
      i2c1 scl -PB8
      i2c1 sda -PB9
      
      i2c4 scl -PF14
      i2c4 sda -PF15

DMA: 
  SPI1_RX DMA2 STREAM2
  SPI1_TX DMA2 STREAM3
  
  UART5_RX DMA1 STREAM0
  UART5_TX DMA1 STREAM7
  
  ADC1 DMA2 STREAM4



Software requirements: 
  cubeIDE program. 

  
  **Notice, the code is seperted into files,locating inside proj directory.
  for running the code, enter to Core directory ->Src directory and and run the main.c file. 

Installition: 
  Connect the board to your PC, and attach the requiered jumpers to the suitable GPIO's
  Connect the usb wire to PC and to the board. 
  Open project directory on cubeIDE. 

How to unistall: Deleting the directory which was downloaded on Github.

How to use: 
  For running the code, enter to Core directory ->Src directory and and run the main.c file. 
  Then, run the code of PC testing program(see Final_project_ARM_PC_Testing_Program for more details).

Example of using: 
  After code is ran, it listening to packets comming from PC. 
  When packet arrivas, its data is procceed.
  If the packet icnludes priphery to be checked, suiteable test or tests will take place,else no test takes place. 
  Then, the result is sent back to PC via packed sturcture size of 5 bytes. 
  


