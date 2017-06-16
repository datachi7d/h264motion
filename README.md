h264 Motion
===========

 Displays the motion info that can be seen from running ffplay:

    ffplay -flags2 +export_mvs input.mp4 -vf codecview=mv=pf+bf+bb

 Install prerequisites:

    sudo apt-get install libavcodec-dev libavformat-dev libavutil-dev libsdl2-dev

 Build:

    make
   
 Run:
 
    ./build/src/h264m <file>
