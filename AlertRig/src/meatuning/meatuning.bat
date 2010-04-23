rem orientation full screen
..\..\bin\meatuning -t 0.5 -b gray -d 500 -g 0,0,0,0,100,.3,3.00,90.0,b,s,e -O 0,60,120,180,240,300,360

rem orientation NOT full screen
rem ..\..\bin\meatuning -t .5 -b gray -d 500 -g 0,0,10,10,100,.3,3.00,90.0,b,s,e -O 0,90,180,270,360

rem spatial frequency full screen
rem C:\work\AlertRig\bin\meatuning -t 1.0 -b gray -d 500 -g 0,0,0,0,100,.3,3.00,90.0,b,s,e -S 0.1,.5,1,2,3

rem temporal frequency full screen
rem C:\work\AlertRig\bin\meatuning -t 1.0 -b gray -d 500 -g 0,0,0,0,100,.3,3.00,90.0,b,s,e -T 0.1,.5,1,2,3

rem contrast full screen
rem C:\work\AlertRig\bin\meatuning -t 1.0 -b gray -d 500 -g 0,0,0,0,100,.3,3.00,90.0,b,s,e -C 1,1.35936,1.84785,2.51189,3.41455,4.64159,6.30957,8.57696,11.6591,15.8489,21.5443,29.2864,39.8107,54.117,73.5642,100

rem orientation tuning full screen
rem C:\work\AlertRig\bin\meatuning -t 1.0 -b gray -d 500 -g 0,0,0,0,100,.3,3.00,90.0,b,s,e -O 0,60,120,180,240,300,360

rem xy test full screen
rem C:\work\AlertRig\bin\meatuning -t 4.0 -b gray -d 500 -g 0,0,0,0,100,.3,0.00,90.0,b,s,e -X 0,90,180,270 -x 0.25

rem orientation, with square aperture
rem d:\work\AlertRig\bin\meatuning -t 1.0 -b gray -d 500 -g 0,0,200.0,200.0,100,.3,3.00,90.0,b,s,e -O 0,60,120,180,240,300,360

rem try two in a row
rem C:\work\AlertRig\bin\meatuning -t 1.0 -b gray -d 500 -g 0,0,0,0,100,.3,0.00,90.0,b,s,e -X 0,90,180,270 -x 0.25
rem C:\work\AlertRig\bin\meatuning -t 1.0 -b gray -d 500 -g 0,0,0,0,100,.3,3.00,90.0,b,s,e -T 0.1,.5,1,2,3

rem use an input spedc file
rem ..\..\bin\meatuning -v -t 1.0 -b gray -d 500 -u 4.0 -f meatlist.txt
