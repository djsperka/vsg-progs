loomie  - Displays blank screen with user-specified luminance value (0-1). 
          Can specify master (M) or slave (S) screen (if rig has master/slave
          setup) or default (D) if not. Prompt asks for a luminance value between
          0 and 1. The screen luminance is set to that value immediately. 


Command Line options:

-D	use default VSG (this is the default)
-M	use master VSG
-S	use slave VSG


This program is for luminance testing only. The notion of "master" and "slave" are 
determined by settings in the registry. The relevant Spike2 registry settings are 
found under HKCU/Software/CED/Spike2. The value of the key "VSGMaster" points to 
a vsg configuration file found in the folder
C:\Program Files\Cambridge Research Systems\VSGV8
Its possible that a relative path (containing "Configurations\" for example) may be
used. 