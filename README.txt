----------------------------
Remote Attestation version: ra 1.4
----------------------------
Purpose of RemoteAttestation
----------------------------
The project demonstrates:
- How an application enclave can attest to a remote party
- How an application enclave and the remote party can establish a secure session

------------------------------------
How to Build/Execute the Sample Code
------------------------------------
1. Install Intel(R) SGX SDK for Linux* OS
2. Build the project with the prepared Makefile:
    a. Hardware Mode, Debug build:
        $ make
    b. Hardware Mode, Pre-release build:
        $ make SGX_PRERELEASE=1 SGX_DEBUG=0
    c. Hardware Mode, Release build:
        $ make SGX_DEBUG=0
    d. Simulation Mode, Debug build:
        $ make SGX_MODE=SIM
    e. Simulation Mode, Pre-release build:
        $ make SGX_MODE=SIM SGX_PRERELEASE=1 SGX_DEBUG=0
    f. Simulation Mode, Release build:
        $ make SGX_MODE=SIM SGX_DEBUG=0
3. Execute the binary directly:
    $ ./app
4. Remember to "make clean" before switching build mode

-------------------------------------
Usage of this sample
-------------------------------------
1.ISV :
	a.ISV work as a server,listen at localhost:84(this can be changed in isv_app/isv_app.cpp)
	b.if you meet the / permission denied / ,please use 
		$sudo ./ISV

2.ServiceProvider
	a.ServiceProvider work as a client
	b.you should input ServiceProvider IP PORT (*message)
		$./ServiceProvider 1 84 


