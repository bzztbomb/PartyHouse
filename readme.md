# PARTY HOUSE!

## Summary

Here's the code for the application I used for "Party House" at the DorkbotPDX Open Mic Surgery event (http://dorkbotpdx.org/wiki/open_mic_surgery_2012_12_17).  Party House is a project that uses a doll house as a target for projection mapping.  This application allows one to create quads and map from an input texture.  

This was written over 3 days, so it's not fancy.  It has the bare minimum of features I needed to perform.

## Features

* Edit mode for input quads
* Edit mode for output quads
* Performance mode
* Loading and saving mapping
* Randomly colored quads
* A roof quad that can contain: a test pattern, "vu meter", pulsed sin blob
* Plays movies (dancers and mice)

## Use / caveats
It expects its source data in the BASE_PATH, so download this data:  http://depot.knowhere.net/party_house_data.zip put it somewhere and update the constant in hausmap_App.cpp.  Output mapping is dependent on screen coordinates, so it's not too graceful with screen resolution changes.

Keyboard shortcuts are:
* F1 - Switch modes
* "F" full screen/windowed toggle
* "Z" In output edit mode it will lay out the quads to match the input quads
* "A" add a surface
* "S" save mapping
* "L" load mapping
* Delete - delete current surface
* "C" clear input layers
* "1" load alignment image
* "2" load glow movie
* "3" color blocks
* "4" test pattern on roof
* "5" vu meter on roof
* "6" sin blob on roof
* "7" dancers movie
* "8" mid section vu meter
* "9" mice movie
* Space bar change colors

## Code
The basic flow is the app renders to an offscreen FBO in hausmap_App::update, then uses that to render the output objects in hausmap_App::render.  Maybe not efficient, but allows for easy compositing. ;)

The code also relies on the wonderful Cinder library.  Clone it from https://github.com/cinder/Cinder and update the CINDER_PATH variable in the xcode project. 

Also, I've only tested/built this with XCode.   The Visual Studio projects have not been updated, but I imagine the code would work on Windows without a lot of pain.

bzztbomb (Brian Richardson)
http://bzztbomb.com/