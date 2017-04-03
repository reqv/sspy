# SSpy
SSpy is a keylogger based on the Windows system services and not hooks. Once installed as a service, he is going to capture all data provided by any user via keyboard.
Program was tested on Windows 7 and was written in Visual Studio 14.

### How it works
Application is split between 2 programs. SSpy is a base application that handle service operations such as install, remove service and he also makes sure that Spy app works correctly. When SSpy start working, he will copy Spy program to ```C:\\``` folder. Spy application is running by SSpy and it job is to handle and save all keyboard input provided by user to ```log.dat``` file located at ```C:\\```. 

### Compilation
Compilation is handled manually for now. To compile this program you need **Windows Vista or later and a C compiler**.

### Installation
Make sure that SSpy and Spy is compiled and in the same folder. Open CMD.exe console and write ```SSpy.exe install``` command to install service. Service will be ready to work after reboot.

### Other Commands
```remove``` can remove SSpy from service list.<br>
```version``` show you details about program and author.<br>
```help``` provide help for application. (not yet implemented)<br>
