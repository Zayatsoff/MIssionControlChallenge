



                                                              Lior Rozin
                                                            24 July 2022
                                                                        


                    Height Sensing Subsystem (HSS) in C++
                           

Table of Contents

   1.  Description . . . . . . . . . . . . . . . . . . . . . . . . .   1
   2.  Setup instructions  . . . . . . . . . . . . . . . . . . . . .   1
   3.  Assumptions . . . . . . . . . . . . . . . . . . . . . . . . .   2
   4.  Process . . . . . . . . . . . . . . . . . . . . . . . . . . .   2
     4.1.  Language  . . . . . . . . . . . . . . . . . . . . . . . .   2
     4.2.  Structure . . . . . . . . . . . . . . . . . . . . . . . .   2
     4.3.  Algorithms  . . . . . . . . . . . . . . . . . . . . . . .   2

1. Description

   This is an implementation of the HSS in C++. It has been written on 
   GNU gdb (Ubuntu 12.0.90-0ubuntu1) 12.0.90 to work on Linux x86_64 
   systems. All libraries used are standard internal library headers. 
   HSS receives data sent by MoonWire via UDP packets. Each time HSS 
   receives a "LASER_ALTIMETER"(0xAA01) message, it sends a "HEIGHT"
   (0xAA31) message back to MoonWire containing the current height of 
   the Lunar lander in centimetres. Once the Lunar lander has landed, 
   an "ENGINE_CUTOFF"(0xAA11) message is sent to MoonWire.

2.  Setup instructions  

   You may either compile 'hss.cpp' or run the pre-compiled 'hss' file.
   To run the pre-compiled 'hss' file:

	1. Navigate to the file via the Linux terminal.
	2. Run the file by running ` $ ./hss `.
	3. After running the file, use the simulator with 
	   ` $ ./build/simulator ` to begin. 

   To compile 'hss.cpp', you can use any C++ compiler. For example,
   to compile with g++:

	1. Navigate to the file via the Linux terminal.
	2. Compile using ` $ g++ -o compiled_hss hss.cpp `.
	3. Run the file by running ` $ ./compiled_hss `.
	4. After running the file, run the simulator with 
           ` $ ./build/simulator` to begin. 

   The 'hss' file will output all needed information onto the terminal.
   Once the spacecraft has landed, 'HSS' will output: 
   	"The Lunar lander has landed!" 
   It will then exit out of the program.


Rozin                                                           [Page 2]

                    Height Sensing Subsystem (HSS)             July 2022


3. Assumptions

   1. The raw sensor data can be directly translated from 
      0-65535(UINT16_MAX) to 0-100,000(1000m in cm). At 40.0cm, the
      spacecraft has landed, and if the measurements are below 40cm,
      then the spacecraft is broken.

   2. Due to debris, the measurements taken by the three sensors may 
      differ. By that assumption, the average of the three measurements
      taken by the sensors define the landing. Once the average is 40cm
      or below, the HSS can send the "ENGINE_CUTOFF" message.

   3. The "HEGIHT" message is sent in 10 bytes, since no more than 10 bytes
      are needed.

4. Process
 
4.1.  Language

   I started writing in Python since it is my preferred language. 
   Python is quite versatile and reliable; it allows for cross-platform 
   development, meaning that I could develop on Windows and still have 
   it work on Linux. Python also has many of the needed functions built in,
   allowing me to bind to a socket using two lines of code and convert and 
   unpack the UDP packets with one. Python also offers automatic memory 
   management, meaning I don't need to worry about memory. Unfortunately,
   I couldn't correctly pack all the bytes in Python when sending them back 
   to MoonWire since I couldn't use external libraries.

   Considering that the code needs to be run on a subsystem--runtime is vital.
   C++'s low-level capabilities allow for efficient and swift runtime. C++ also
   allows for error handling, which is very important since it simplifies the 
   life of not only the user but also the developer by making the code easier 
   to maintain. 

4.2.  Structure

   * All the code is located in one file not only because of simplicity but 
     also due to the small number of lines needed. 

   * The file is documented from within, allowing for an easy debugging and 
     reading experience.  

   * Many repetitive tasks have either been assigned to functions or run within 
     a loop allowing for optimal file size.

   * Sensor data and readings have been assigned to arrays, allowing for the 
     use of loops as well as making maintenance easier for future developers.

4.3.  Algorithms

   The following algorithm was used to convert the raw data into centimetre
   measurements:

	` Height = (100000*raw_value)/65535 `

   In order to convert to and from Big Edians bitwise operators were used to
   reverse the byte order to decrypt the data.

	uint16_t : ` Swapped = (val  <<  8) | (val  >>  8) `

	uint32_t : ` val = val = ((val << 8) & 0xFF00FF00) 
	                       | ((val >> 8) & 0xFF00FF); 
	             Swapped = (val << 16) | (val >> 16); `
     `
   

