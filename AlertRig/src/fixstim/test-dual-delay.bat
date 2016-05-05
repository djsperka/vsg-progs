REM local
work\AlertRig\bin\remote.exe 127.0.0.1 7000 fixstim -f 0,0,1,red -b gray  -p 2  -V 128,FS -s 5,0,3,3,0,0,100,.4,4,90,180 -E 100,0,50,0,200,0 -C 50,60,70,80,90,100 -O 0,45,90,135,180 -v -d 500

REM remote
work\AlertRig\bin\remote.exe 127.0.0.1 7000 fixstim -f 0,0,1,red -b gray  -p 2  -V 128,FS -s 5,0,3,3,0,0,100,.4,4,90,180 -E 100 -C 50,60,70,80,90,100 -O 0,45,90,135,180 -v -d 500