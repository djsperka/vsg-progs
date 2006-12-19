rem  run tuning area
rem C:\work\AlertRig\Spike2Scripts\..\bin\Tuning  -a -f 0,0,0.25,blue -b gray -d 500 -g -10.8,-5.7,5.0,5.0,-100,0.50,4.00,90.0,b,s,e -A 0.15,0.208299,0.289258,0.401682,0.5578,0.774597,1.07565,1.49372,2.07427,2.88047,4
rem same thing without ascii triggers - use spike2, run sampling config sequencer file...
rem C:\work\AlertRig\Spike2Scripts\..\bin\Tuning  -f 0,0,0.25,blue -b gray -d 500 -g -10.8,-5.7,5.0,5.0,-100,0.50,4.00,90.0,b,s,e -A 0.15,0.208299,0.289258,0.401682,0.5578,0.774597,1.07565,1.49372,2.07427,2.88047,4

rem orientation tuning here , ascii triggers
rem C:\work\AlertRig\Spike2Scripts\..\bin\Tuning -a -f 0,0,0.25,blue -b gray -d 500 -g -1.9,-11.1,10.0,10.0,-100,1.00,0.00,90.0,b,s,e -O 0,90,180,270,360

rem orientation tuning here , use sampling sequencer
rem C:\work\AlertRig\Spike2Scripts\..\bin\Tuning -f 0,0,0.25,blue -b gray -d 500 -g -1.9,-11.1,10.0,10.0,-100,1.00,0.00,90.0,b,s,e -O 0,90,180,270,360


rem SANDBOX - orientation tuning here , ascii triggers
rem C:\work\AlertRig\Spike2Scripts\..\bin\sandbox -a -f 0,0,0.25,blue -b gray -d 500 -g -1.9,-11.1,10.0,10.0,-100,1.00,1.00,90.0,b,s,e -O 0,90,180,270,360

rem sandbox, orientation, spike2 triggers
rem C:\work\AlertRig\Spike2Scripts\..\bin\sandbox -f 0,0,0.25,blue -b gray -d 500 -g -1.9,-11.1,10.0,10.0,-100,1.00,2.00,90.0,b,s,e -O 0,90,180,270,360

rem sandbox, area, spike2 triggers
rem C:\work\AlertRig\Spike2Scripts\..\bin\sandbox  -f 0,0,0.25,blue -b gray -d 500 -g -10.8,-5.7,5.0,5.0,-100,0.50,4.00,90.0,b,s,e -A 0.15,0.208299,0.289258,0.401682,0.5578,0.774597,1.07565,1.49372,2.07427,2.88047,4

rem SANDBOX - spatial tuning here , ascii triggers
rem C:\work\AlertRig\Spike2Scripts\..\bin\sandbox -a -f 0,0,0.25,blue -b gray -d 500 -g -1.9,-11.1,10.0,10.0,-100,1.00,1.00,90.0,b,s,e -S 0.1,.5,1,2,3
rem C:\work\AlertRig\Spike2Scripts\..\bin\sandbox -a -f 0,0,0.25,blue -b gray -d 500 -g -1.9,-11.1,10.0,10.0,-100,1.00,1.00,0.0,b,s,e -S 0.1,.5,1,2,3
rem C:\work\AlertRig\Spike2Scripts\..\bin\sandbox -a -f 0,0,0.25,blue -b gray -d 500 -g -1.9,11.1,10.0,10.0,-100,1.00,1.00,0.0,b,s,e -S 0.1,.5,1,2,3

rem sandbox - temporal ascii
rem C:\work\AlertRig\Spike2Scripts\..\bin\sandbox -a -f 0,0,0.25,blue -b gray -d 500 -g -1.9,11.1,10.0,10.0,-100,1.00,1.00,0.0,b,s,e -T 0.1,.5,1,2,3

rem sandbox - contrast ascii
C:\work\AlertRig\Spike2Scripts\..\bin\sandbox -a -f 0,0,0.2,red -b gray -d 700 -g 6.4,2.7,2.0,2.0,100,2.00,4.00,120.0,b,s,e -C 1,1.35936,1.84785,2.51189,3.41455,4.64159,6.30957,8.57696,11.6591,15.8489,21.5443,29.2864,39.8107,54.117,73.5642,100
