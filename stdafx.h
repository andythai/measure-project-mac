// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#pragma once

#include <stdio.h>



// TODO: reference additional headers your program requires here
#include <string>
#include <iostream>
#include <fstream>
#include <sys/stat.h>
#include <vector>
#include "Coordinates.h"
#include <cmath>

// Hide OpenCV documentation warnings
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdocumentation"
#pragma clang pop

// OpenCV 3.10.3
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
