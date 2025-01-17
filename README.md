## LunixTNG: A Wireless Sensor Network Driver on the Linux operating system 

<img width="639" alt="Screenshot 2024-11-20 at 11 23 13 PM" src="https://github.com/user-attachments/assets/b4755865-0c37-4d72-8159-845330ef3a1d">

The wireless network we worked with has a number of sensors (Crossbow MPR2400CA wireless cards with MDA100CB voltage, temperature and brightness sensors) and a base station (MPR2400CA card and MIB520CB USB interface). The base station is connected via USB to a Linux computer system on which the requested device driver is executed.

The sensors periodically report the result of **three** different **measurements**: the **voltage** of the battery that powers them, the **temperature** and the **brightness** of the room where they are located. 

### Example of using the driver

The simple user "user", after logging in, compiles the code of the module by issuing the make command:

<img width="613" alt="Screenshot 2024-11-20 at 11 34 04 PM" src="https://github.com/user-attachments/assets/cc36eaea-1dfb-43e9-886f-acfd7f777782">



It then gains administrator (su) privileges to load the module (insmod), create a number of device-specific files (mk-lunix-devs.sh) and enable data input from the USB serial port /dev/ttyUSB1 (with lunix-attach), where we assume the wireless sensor network base station is connected.

<img width="624" alt="Screenshot 2024-11-20 at 11 34 43 PM" src="https://github.com/user-attachments/assets/94e8247b-1564-47c3-ab0b-66da4c380b1e">

**Finally**, it asks for the temperature of the 3rd sensor.
<img width="333" alt="Screenshot 2024-11-20 at 11 35 35 PM" src="https://github.com/user-attachments/assets/34095819-77f4-4330-a2e3-ddfcd2785c6c">
And the results would appear as a series of measurements like:

**27.791**[small pause for acquiring the measurement] **27.791 27.693 27.791 ^C**

The implementation of the code for the 

#### In the implementation of this project, along with the understanding of how a driver works, we got (more)familiar with concepts like Process και Interrupt Context,Synchronization methods, safe data transfer to user space etc.

The files that implement the basic driver's operations are:
```arduino
lunix-chrdev.h
```
and 
```arduino
lunix-chrdev.c
```
### System Requirements

To run the driver, ensure that the following system requirements are met:

  #### Hardware Requirements:
- A machine (physical or virtual) with at least one available serial port.
- (Optional for laboratory setup) Access to a real sensor connected to /dev/ttyUSB1 on the host machine.
  
### Software Requirements:
- **QEMU-KVM:** Virtual machine software capable of emulating a serial port and running the driver inside the VM.
Ensure QEMU-KVM is installed and properly configured on your system.
- **Linux Operating System:** The system running the driver must be running a Linux-based OS (Ubuntu, CentOS, etc.).
- **Driver Dependencies:** Any libraries or kernel modules required by the driver should be installed (refer to the driver documentation for details).
  
### Network Requirements:
If you are working outside the laboratory, you need network access to **lunix.cslab.ece.ntua.gr** on **port 49152 for the remote sensor data**.
    
### How to Run the Driver

**1. On the CS Laboratory Machine (with a Real Sensor)**

To run the driver on a physical system, the base station must be connected to a Serial-over-USB device (e.g., /dev/ttyUSB1). 

  - If you are running this on a laboratory machine and have access to the real sensor, follow these steps:

    - **Mapping the Serial Port:** When running in a QEMU-KVM virtual machine, map the host's /dev/ttyUSB1 to the second virtual serial port (/dev/ttyS1) inside the VM.

      - **QEMU-KVM Command:** Use the following command line parameter to map the serial port:
        
```bash
-chardev tty,id=sensors0,path=/dev/ttyUSB1
```
**Driver Behavior:** Once this mapping is set up, any access to the virtual serial port /dev/ttyS1 inside the QEMU-KVM VM will correspond to the physical /dev/ttyUSB1 port on the host system. The driver will operate as expected, and there is no need to modify the device path in the example code.

 #### 2. On a Machine Outside the Laboratory (e.g., Personal Machine or Cloud) -- An ssh connection to the lab server is needed

If you are developing outside the laboratory environment (e.g., using QEMU-KVM on your personal machine or in the cloud), and you do not have direct access to a physical sensor, you can still test the driver by connecting to a remote TCP/IP server that broadcasts measurements from a base station.

- **Accessing the Remote Sensor Data**: The server is hosted on **lunix.cslab.ece.ntua.gr** and continuously transmits sensor data over TCP port 49152.

- **Automated Connection**: On the client side (inside your QEMU-KVM virtual machine), you need to connect the second virtual serial port (/dev/ttyS1) to the TCP server. This is automatically handled by the utopia.sh script provided.

- **Running the QEMU-KVM Command:** When you run QEMU-KVM, the utopia.sh script will redirect the virtual serial port /dev/ttyS1 to the remote TCP server at lunix.cslab.ece.ntua.gr:49152. You do not need to manually configure anything for this to work.


