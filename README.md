# SSpy
**Current version: 0.32 alpha**

SSpy is a keylogger based on the Windows system services and not hooks. Once installed as a service, he is going to capture all data provided by any user via keyboard.

Program was tested on Windows 7 and was written in Visual Studio 14.

### More about application
Application is split between 2 programs. SSpy is a base application that handle service operations such as install, remove service and he also makes sure that Spy app works correctly. When SSpy start working, he will copy Spy program to `C:\\` folder. Spy application is running by SSpy and it job is to handle and save all keyboard input provided by user to `log.dat` file located at `C:\\`. 

You are using this application at your own risk.

### Working On
- [ ] delete hardcoded paths
- [ ] support for win10
- [ ] major improvements to Spy application
- [ ] help

### How to use this application

##### Compile
Compilation is handled manually for now. To compile this program you need **Windows Vista or later and a C compiler**.

##### Install / Remove service
1. Make sure that SSpy and Spy is compiled and in the same folder. 
2. In the folder with applications open CMD.exe console and write `SSpy.exe install` command to install service.
3. Service will be ready to work after reboot.

To uninstall service, in step 2 write command `SSpy.exe remove`