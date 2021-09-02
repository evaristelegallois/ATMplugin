### Automatic Throw Measurement (ATM) plug-in

## description
The ATM plug-in was designed to automatically compute fault displacement from 3D scarp data. This CloudCompare plug-in was developped by G. Parel during a 5-months internship in CEREGE, Marseille, under the supervision of S. Viseur.

CloudCompare is an open source 3D point cloud processing software created by D. Girardeau.

## installation guide
1/ Download the "qThrowMeasurement" folder and copy it in the CloudCompareplugins/core/Standard directory. Add the line "add_subdirectory( qThrowMeasurement )" in the "CMakeLists.txt" before the plug-ins are set.

2/ Download the QtCharts package from Qt website. Version should be the same as your current Qt installation (if Qt isn't installed on your computer, download Qt as well, 5.9.9 minimum).

3/ Depending on your preferred way of building CloudCompare, there are two options here:

# OPTION 1: building CloudCompare via CMake
Follow the CloudCompare build & install via CMake instructions (see https://github.com/CloudCompare/CloudCompare/blob/master/BUILD.md for further instructions). 

The "Plug-in ATM standard" box should be checked-on to proceed with installation. When prompted, edit the QtCharts' directory line as well.

After configurating and generating the build, open the project in Visual Studio. If building/installing fails, edit the "CMakeLists.txt" in CloudCompare/plugins/core/Standard/qThrowMeasurement and comment the following line "find package (Qt5Charts)". 

Generate "BUILD_ALL" then "INSTALL". Go to the "CloudCompareProjects" directory (Visual Studio's log tells you where it saved it if you didn't edit the path yourself) > CloudCompare_debug > CloudCompare.exe.

# OPTION 2: building CloudCompare directly via Visual Studio
Open the "CloudCompare" directory in Visual Studio. Right-click on the cMakeLists.txt located in said directory > Generate Cache. Building will fail; right-click on cMakeLists.txt > CMake Cache > Display CMakeCache.txt.

Look for "NOT_FOUND" command lines: those are the directories you need to manually add (as you would using CMake in Option 1) to generate the project. 

If building still fails after this step:
1/ Check that the "find package (Qt5Charts)" lines in "CMakeLists.txt" in CloudCompare/plugins/core/Standard/qThrowMeasurement/CMakeLists.txt is NOT commented.
2/ Go to CloudCompare/out/install/x-64 Debug/CloudCompare. If "QtCharts5.dll" and "QtCharts5d.dll" are missing, you need to manually add them.

Now, building should proceed as intended. You can now choose to run CloudCompare directly from the "Run" command by selecting "CloudCompare.exe (Installer)" in the "Run" list. 

4/ See user guide "ATM2D_userGuide.pdf" for further instructions on how to use the ATM plug-in.

## known issues
1/ When computing throw measurement two times in a row, even if p value was changed, the first p value is used anyway. 
Fix-it: In atmdialog.cpp, an offset needs to be added to the computeThrowMeasurement() function in the first two for loops (i = 0 -> i < m_profiles.size() should become i = 0 + value*m_profiles.size() -> i < value*m_profiles.size(), with value starting @ 0 and being incremented at each call of computeThrowMeasurement()) and the loading of values in the m_y list edited accordingly.

2/ Profile IDs should be replaced by y position on generatrix in the displacement graph.
Fix-it: In atmdialog.cpp, computeThrowMeasurement(): replace the values in "x" list by the y position on generatrix. Those can be obtained in the profileProcessor.cpp, where intercept with generatrix is computed.

3/ After clicking on "Computing throw measurement" in main plug-in dialog, sometimes a second click during computation induces a CloudCompare crash.
Fix-it: ??? Trying to implement a progress dialog to lock user input wasn't sucessful (seems to need multi-threading?). Might be interesting to lock user input without the progress dialog.

## future upgrades
1/ The createLineGraph() in atmDisplayProfilesDlg.cpp could be improved by splitting the clustering/segmentation display in two different slots. Those could then be called by checking/unchecking the corresponding boxes.

2/ Adding const and static references. It was poorly if not done at all.

3/ Adding more display options in the secondary plug-in dialog as well as returning clustering data if needed.

4/ Adding more segmentation methods. See the computeSegmentation() function in DPPiecewiseLineartRegression.cpp for more details.

5/ Computing fault displacement related to an arbitrary X axis, which could be used to generate data cumulative displacement data (from different faults located close to each other). See the createLineChart() function in atmDialog.cpp for more details.