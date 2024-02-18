copy Flash\Obj\output.hex ..\output(mdk).hex
start /b Keil5_disp_size_bar_v0.3.exe
@ECHO OFF
D:\Keil_v5\ARM\ARMCC\bin\fromelf.exe --bin -o ..\output.bin Flash\Obj\output.axf"

