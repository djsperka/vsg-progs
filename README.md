# vsg-progs
library and collection of visual stimulus programs written for the VSG and Visage from Cambridge Research Systems (http://crsltd.com)

This represents 15+ yrs of development of visual stimuli using the Visage system from CRS (and its prececessor the VSG 2/5). 
As such, there is quite a bit of old abandoned code which I will be clearing out and removing. 

# setup
- install boost
  - installation is confusing. Need header-only libs as well as compiled libs. Be a hero and build it yourself, or else get [pre-built binary download.](https://boost.teeks99.com/)
- install visage stuff
- install asl sdk (unless no eyetracking)
- set env variables
  - BOOST_INCLUDE_PATH, BOOST_INCLUDE_PATH_64 - path to folder where you can find "boost/version.hpp". Address include files with <boost/xyz.hpp> in code
  - BOOST_LIB_PATH, BOOST_LIB_PATH_64 - path to folder containing boost_*.lib (e.g. "boost_fileystem-vc141-mt-gd-x64-1_69.lib"
  - VSG_INCLUDE_PATH, VSG_INCLUDE_PATH_64 - path to folder containing "VSGV8.H"
  - VSG_LIB_PATH, VSG_LIB_PATH_64 - path to folder containing vsgv8.lib
  - ASL_INCLUDE_PATH, ASL_INCLUDE_PATH_64 - path to folder containing "ASLSerialOutLib2.h" - they put it in same folder as dll
  - ASL_LIB_PATH, ASL_LIB_PATH_64 - path to folder containing ASLSerialOutLib2.dll
  - ALERT_DEPLOYMENT_ROOT - path to root of rig installation. Exe files are written to "bin" or "bin64" folder here. 

# building
- MSVC 2017, newer versions not tested. 
- Choose correct project config
- Build alertlib first (if build entire solution, some parallel builds will fail because this isn't done yet, and I haven't bothered with the config to fix that)
- Build fixstim, cabletest, remote, sendMessage, MsgSvr
- Build "aaaDeploy" to move exe files to bin folder used in spike2 scripts

# notes

Building these exe files and libs (with the exception of "aaaDeploy") is guaranteed NOT to affect production stuff (just make sure "vsg-progs" and "usrig" folders are not nested at all). 

Since we rely heavily on "fixstim", you can run it from any folder for testing. The scripts can communicate with its port regardless of where "fixstim.exe" resides. 

  