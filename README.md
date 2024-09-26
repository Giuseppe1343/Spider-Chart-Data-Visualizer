# Spider Chart Data Visualizer
> A c++ game developed to understand the basics of multi-threaded programming.

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT) ![Environment: Windows](https://img.shields.io/badge/Environment-Windows-blue) 

<div align="center">
  <img src="https://github.com/Giuseppe1343/Spider-Chart-Data-Visualizer/blob/main/charts.gif"/>
</div><be>


### Table of contents
- [Introduction](#introduction)
- [Features](#features)
- [Code Overview](#code-overview)
- [Dependencies](#dependencies)

  
#  Introduction

This project is a C++ application that generates a spider chart (also known as a radar chart) using custom graphics functions. This application dynamically creates a spider chart and aims to make it easier for you to analyze multiple data on this chart. No data visualization library was used. Created from pixels and custom algorithms.

> [!IMPORTANT]
>  [ICBYTES](https://github.com/cembaykal/ICBYTES) Library is not a data visualization library.

# Features

- Draw pixels with optional transparency
- Draw lines between points
- Fill rectangles with a specified color and transparency
- Draw and fill polygons using the Scan Line Algorithm
- Draw and fill polygons using the Nonzero Winding Number Rule
- Generate spider charts with customizable categories and values

# Code Overview

## Functions

### `Create_New_Color()` 
Blends two colors with a given alpha value.
> Takes two colors (current_color and rgb_color) and an alpha value (transparency). It blends the two colors based on the alpha value and returns the new color.

### `Draw_Pixel()` 
Draws a pixel on the image with optional transparency.
> Sets the color of a specific pixel in the image. It can blend the new color with the existing color based on the alpha value.

### `Draw_Line()` 
Draws a line between two points.
> Uses the Bresenham's line algorithm to draw a line between two points (x1, y1) and (x2, y2) on the image.

### `Fill_Rectangle()` 
Fills a rectangle with a specified color and transparency.
> Fills a rectangular area on the image with a specified color. It can also apply transparency using the alpha value.

### `Draw_Polygon()`
Draws a polygon by connecting a series of points.
> Draws a polygon by connecting a series of points provided in the `points` matrix. It also marks the corners with small rectangles.

### `Fill_Polygon()` 
Fills a polygon using the Scan Line Algorithm.
> Fills the interior of a polygon using the Scan Line Algorithm. It finds intersection points of scan lines with polygon edges and fills between pairs of intersections.

### `Fill_Polygon_Nonzero_Winding_Number_Rule()` 
Fills a polygon using the Nonzero Winding Number Rule.
> Fills the interior of a polygon based on the winding number rule. It calculates the winding number for each pixel and fills the pixel if the winding number is nonzero.

### `Draw_Spider_Chart_Core()`
Draws the core of the spider chart, including the legs connecting data points to the center.
> Draws the main structure of the spider chart, including the lines (legs) connecting data points to the center of the chart.

## Data Structures

### `Button`
Represents a button with an ID, color, handle, image, and value handle.
> Used to manage buttons in the GUI, including their appearance and behavior.

### `ICBYTES` 
Handles image data and other byte-related operations.
> Provides various methods for manipulating image data, including setting pixel values, getting dimensions, and performing arithmetic operations on the data.


# Dependencies

- Standard C++ libraries.
- [ICBYTES](https://github.com/cembaykal/ICBYTES) library.
