rem C:\Users\lab\Desktop\work\dan\AlertRig\src\dualstim>copy /y ..\..\bin\dualstim.exe z:\dan\AlertRig\bin

rem dualstim.exe -a -d 1000 -b gray -r f:\experiments\offset.txt -f 0,0,1,red -P 180 -K -M -s 0,0,5,5,100,.2,0,90 -A 1,2,4,8 -V -s 0,0,5,5,100,.5,1,0 -H 4,1,8,1,6,5,3,0

rem C:\Users\lab\Desktop\work\dan\AlertRig\bin>c:\sysinternals\psexec -c dualstim.exe \\128.120.140.202 -a -b gray -d 1000 -r 0,0 -K -s 5,5,5,5,100,.2,.1,90,b,s,e -M -A 1,3,5 -s -5,-5,5,5,100,.2,.1,180,b,s,e -V -A 3,5,7 -v
c:\sysinternals\psexec -c ..\..\bin\dualstim.exe \\128.120.140.202 -a -p 2 -b gray -d 1000 -r 0,0 -K -f 0,0,0.5,red -M -g 5,5,5,5,100,.2,.1,90,b,s,e -V -g -5,-5,5,5,100,.2,.1,180,b,s,e -v

