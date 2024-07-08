# ns3-Satellites-Ground-Network-Simulation
Simulating satellite-ground network using ns-3 on Ubuntu18.04.

## Basic Settings
- Ubuntu 18.04
- ns-3.37

## Prerequisite
- Before compling this ns3 project, you need to PIP necessary files.
```
sudo apt install git 
sudo apt-get install g++ python3 python3-dev pkg-config sqlite3 cmake
sudo apt-get install mercurial
sudo apt-get install qt5-default
sudo apt-get install autoconf cvs bzr unrar
sudo apt-get install gdb valgrind
sudo apt-get install gsl-bin libgsl-dev libgslcblas0
sudo apt-get install flex bison libfl-dev
sudo apt-get install g++-3.4 gcc-3.4
sudo apt-get install tcpdump
sudo apt-get install sqlite sqlite3 libsqlite3-dev
sudo apt-get install libgtk2.0-0 libgtk2.0-dev
sudo apt-get install libgtk-3-dev
sudo apt-get install libxml2 libxml2-dev
sudo apt-get install vtun lxc uml-utilities
sudo apt-get install uncrustify
sudo apt-get install doxygen graphviz imagemagick
sudo apt-get install texlive texlive-extra-utils texlive-latex-extra texlive-font-utils dvipng latexmk
sudo apt-get install python3-sphinx dia 
sudo apt-get install gir1.2-goocanvas-2.0 python3-gi python3-gi-cairo python3-pygraphviz gir1.2-gtk-3.0 ipython3
sudo apt-get install libxml2 libxml2-dev libboost-all-dev
sudo apt-get install openmpi-bin openmpi-common openmpi-doc libopenmpi-dev
```

## How to run
(1) Download certian c++/shell transcripts in this project, then put them into your own ns-3 project's different floders relatively. Make sure that you put the transcripts into correct folders!
- 'MeshNet_connected_ip2_route_trace_goundnet.cc' should be put into the floder named 'scratch'.
- 'NSH.cc','NSD.cc','LRST.cc' should be put into the floder named 'LinkingLogic_Decision'. 
(If there's no floder named 'LinkingLogic_Decision', you can create one under 'ns-3.37' route)
- 'GS_Satnet_Simulation.sh' should just be put under 'ns-3.37' route.

(2) **Run Algorithm:** Run the algorithm transcript(NSH.cc or NSD.cc or LRST.cc), then you'll get output files '_sat1.txt' & '_sat2.txt'.

(3) **Choose the Algorithm:** 'GS_Satnet_Simulation.sh' line20-29, you can choose the algorithm you want to simulate.

(4) **Run shell:** Open the terminal under the route 'ns-3.37', then:
'''
bash GS_Satnet_Simulation.sh
'''

## How to tailor

(1) If you want to change the satellite-ground topology: Read ns-3/cpp codes in 'MeshNet_connected_ip2_route_trace_goundnet.cc', and modify it under the framework.

(2) If you want to simulate more algorithm: Write your own algorithm transcript, then put into 'LinkingLogic_Decision' folder. Make sure that your codes can output '_sat1.txt' & '_sat2.txt'.

(3) Other details: You can change them in transcript 'GS_Satnet_Simulation.sh' as you like.


## Output
You can get four valuation output after running the project:
- ave_PLR: average packet loss rate
- ave_AT: average throughput
- ave_E2ED: average end-to-end delay
- ave_E2EJ: average end-to-end jitter

The output is like this:
```
シミュレーション総回数:50。結果はここから書いております。
=======================>>>>FINAL SIMULATION RESULT<<<<=========================
||   ave_PLR：0
||   ave_AT:78.800448Kbps
||   ave_E2ED:1.9218236000e+11ns
||   ave_E2EJ:1.0025300000e+11ns
||   (maxDelay:280405000000.0000000000 ns  minDelay:180152000000.0000000000 ns)
=======================>>>>FINAL SIMULATION RESULT<<<<=========================
```

By the way, **more detailed output**(valuation output under each time's simulation) during the running process is generated as files named 'SIMOUTPUT_Detailed_forread.txt' & 'SIMOUTPUT_foruse.txt'. They includes not only main valuation outputs, but also have sat-ground connecting number. like this:
```
GS(src)----SAT[2]  ----------> SAT[25]----GS(dst)
TotalSent packets = 200
Total Lost Packets = 0
Packet Loss ratio = 0%
Average Throughput = 79.0236Kbps
End to End Delay = +1.80152e+11ns
End to End Jitter delay = +0ns
```


