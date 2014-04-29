rem ..\..\bin\fixstim -a -v -f 0.0,0.0,1.0,blue
rem ..\..\bin\fixstim -a -v -f 0.0,0.0,1.0,blue -b white
rem ..\..\bin\fixstim -a -v

rem ..\..\bin\fixstim -a -v -d 500 -g 0,0,5,5,100,.2,1,45,b,s,e
rem ..\..\bin\fixstim -a -v -d 500 -f 0,0,.5,red -g 0,0,5,5,100,.2,1,45,b,s,e

rem Contrast tuning curve, ascii triggers
rem ..\..\bin\fixstim -a -v -d 500 -f 0,0,.5,red -s 0,0,5,5,100,.2,1,45,b,s,e -C 100,50,10,90

rem sf tuning curve, ascii triggers
rem ..\..\bin\fixstim -a -v -d 500 -f 0,0,.5,red -s 0,0,5,5,100,.2,1,45,b,s,e -S .1,.5,1,2,4

rem orientation tuning curve, ascii triggers
rem ..\..\bin\fixstim -a -v -d 500 -f 0,0,.5,red -s 0,0,5,5,100,.2,1,45,b,s,e -O 0,45,90,135,180

rem area tuning curve, ascii triggers
rem ..\..\bin\fixstim -a -v -d 500 -f 0,0,.5,red -s 0,0,5,5,100,.2,1,45,b,s,e -A 1,2,3,4,5

rem donut tuning curve, ascii triggers
rem ..\..\bin\fixstim -a -v -d 500 -f 0,0,.5,red -s -8,-8,5,5,100,.2,1,45,b,s,e -H 1,0,2,0,3,2,4,3,5,4

rem tf tuning curve, ascii triggers
rem ..\..\bin\fixstim -a -v -d 500 -f 0,0,.5,red -s 0,0,5,5,100,.2,1,45,b,s,e -T 0.1,0.5,1,2,5

rem tf tuning curve, BINARY triggers
REM ..\..\bin\fixstim -v -d 500 -f 0,0,.5,red -s 0,0,5,5,100,.2,1,45,b,s,e -T 0.1,0.5,1,2,5

rem tf tuning curve, ascii triggers, no fixpt
rem ..\..\bin\fixstim -a -v -d 500 -s 0,0,5,5,100,.2,1,45,b,s,e -T 0.1,0.5,1,2,5

rem this will fail - did not specify seq file -F
rem ..\..\bin\fixstim -a -v -d 500 -f 0,0,.5,red -s 0,0,5,5,100,.2,1,45,b,s,e -R 1,0,100

rem this will fail - file not found
rem ..\..\bin\fixstim -a -v -d 500 -s 0,0,5,5,100,.2,1,45,b,s,e -F stim_not_found.txt -R 100,0,10

rem this will fail - sequence file doesn't have that many terms
rem ..\..\bin\fixstim -a -v -d 500 -s 0,0,5,5,100,.2,1,45,b,s,e -F stim.txt -R 100,0,100

rem this will fail - sequence cannot start at term 30 (not enough terms)
rem ..\..\bin\fixstim -a -v -d 500 -s 0,0,5,5,100,.2,1,45,b,s,e -F stim.txt -R 100,30,10

rem this will work
rem ..\..\bin\fixstim -a -v -d 500 -s 0,0,5,5,100,.2,1,45,b,s,e -F stim.txt -R 100,0,10

rem this works, 5 terms, balanced (total terms=10)
rem ..\..\bin\fixstim -a -v -d 500 -s 0,0,5,5,100,.2,1,45,b,s,e -F stim.txt -B 100,1,5

rem this works, 5 terms, balanced (total terms=10), with fixpt
rem ..\..\bin\fixstim -a -v -d 500 -f -5,-5,1,green -s 0,0,5,5,100,.2,1,45,b,s,e -F stim.txt -B 100,1,5

rem crg, using msequence, short one. 
rem ..\..\bin\fixstim -a -v -d 500 -s 0,0,5,5,100,.2,1,45,b,s,e -F ..\..\stim\MSequenceTerms -B 1,100,5
rem ..\..\bin\fixstim -a -v -d 500 -s 0,0,5,5,100,.2,1,45,b,s,e -F ..\..\stim\MSequenceTerms -B 2,100,5

rem CBar
rem ..\..\bin\fixstim -a -v -b black -d 500 -G aaa

rem dots color,x,y,diam,speed,density,dotsize,angle1,angle2,...
rem ..\..\bin\fixstim -a -v -b gray -p 2 -d 500 -f 0,0,1,green -D white,5,5,10,1,10,2,0,45,90

rem danish
rem ..\..\bin\fixstim -a -v -b gray -p 2 -d 500 -f 0,0,1,green -s 0,0,6,6,4,4,100,.2,1,135,b,s,e -s 0,0,4,4,0,0,100,.2,1,45,b,s,e -H 6.0,7.0,8.0
rem ..\..\bin\fixstim -a -v -b gray -p 2 -d 500 -f 0,0,.25,green -s 0,0,6,6,4,4,100,.2,1,135,b,s,e -H 6.0,7.0,8.0

..\..\bin\fixstim -a -f 0,0,0.2,blue -b gray -d 800 -p 2  -v  -s 3.0,3.0,10.0,10.0,5.0,5.0,100,1.00,5.00,160.0,0.0,b,s,e -s 3.0,3.0,2.0,2.0,0.0,0.0,100,1.00,5.00,250.0,0.0,b,s,e -H 10,10
