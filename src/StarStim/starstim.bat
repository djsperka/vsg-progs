rem d:\work\alertrig\bin\starstim -a -b gray -f 0,0,1,r -t 5,0,1,r -n 5 -d 555
rem test offset, second stim should be at 90 deg (directly above fixpt)
rem d:\work\alertrig\bin\starstim -a -b gray -f 0,0,1,r -t 5,0,1,r -n 5 -d 555 -o 18
rem same as offset test above, but fixpt is at 3,3, not 0,0.
rem d:\work\alertrig\bin\starstim -a -b gray -f 3,3,1,r -t 8,3,1,r -n 5 -d 555 -o 18
rem c:\work\alertrig\bin\starstim -b gray -f 0,0,1,r -t 5,0,1,r -n 5 -d 555
rem c:\work\alertrig\bin\starstim -b gray -f 0,0,1,r -t 4,4,1,r -n 7 -d 555
rem c:\work\alertrig\bin\starstim -b gray -f 0,0,1,r -t 4,0,1,r -n 5 -d 555 -r starstim.txt
rem c:\sysinternals\psexec \\192.168.1.1 -c ..\..\bin\starstim -d 500 -b gray -a -v -f 0,0,.5,red -t 5,0,.5,blue -t 5,1,.5,blue -t 5,2,.5,blue -t 5,3,.5,blue -t 5,4,.5,blue -t 5,5,.5,blue 
rem c:\sysinternals\psexec \\192.168.1.1 -c ..\..\bin\starstim -d 500 -b gray -a -v -f 0,0,.5,red -t 5,0,.5,blue -t 5,1,.5,blue -t 5,2,.5,blue -t 5,3,.5,blue -t 5,4,.5,blue -t 5,5,.5,blue -o 5,4,3,2,1,0
D:\work\AlertRig\Spike2Scripts\..\bin\StarStim  -t 5,0,0.5,red  -t 3.06152e-016,5,0.5,red  -t -5,6.12303e-016,0.5,red  -t -9.18455e-016,-5,0.5,red  -o 2,3,1,0 -f 0,0,0.5,blue -p 2 -b gray -d 1000 -a
