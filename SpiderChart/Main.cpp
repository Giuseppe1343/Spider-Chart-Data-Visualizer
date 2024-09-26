#include"icb_gui.h"
#include <stdio.h>
#include <math.h>
#include <vector>
#include <algorithm>
#include <random>
#include <sstream>
#include <iomanip>

//extern ICBDIAG diag; //For debugging

# define M_PI 3.14159265358979323846

int FRM, SLE, TBTN/*, MLE*/;

struct Button
{
	int ButtonId;
	int ButtonColor;
	int ButtonHandle;
	ICBYTES ButtonImage;
	int ValueButtonHandle;
};

Button Buttons[6];

int theme_color = 0;
ICBYTES theme_button_image(__LINE__);

ICBYTES image(__LINE__);
ICBYTES main_data(__LINE__);
ICBYTES data_order(__LINE__);
ICBYTES data_visibility(__LINE__);
ICBYTES value_visibility(__LINE__);



void ICGUI_Create()
{
	ICG_MWSize( /*970*/670, 600);
	ICG_MWTitle("SPIDER-CHART");
}

// Create a new color by blending current_color and rgb_color with alpha
unsigned int Create_New_Color(unsigned int current_color, unsigned int rgb_color, double alpha)
{
	// Extract RGB components from current_color
	unsigned char curr_r = (current_color >> 16) & 0xFF;
	unsigned char curr_g = (current_color >> 8) & 0xFF;
	unsigned char curr_b = current_color & 0xFF;

	// Extract RGB components from rgb_color
	unsigned char rgb_r = (rgb_color >> 16) & 0xFF;
	unsigned char rgb_g = (rgb_color >> 8) & 0xFF;
	unsigned char rgb_b = rgb_color & 0xFF;

	// Scale alpha to [0, 255]
	unsigned char scaled_alpha = static_cast<unsigned char>(alpha * 255.0);

	// Calculate new blended color components
	unsigned char new_r = static_cast<unsigned char>((1.0 - alpha) * curr_r + alpha * rgb_r);
	unsigned char new_g = static_cast<unsigned char>((1.0 - alpha) * curr_g + alpha * rgb_g);
	unsigned char new_b = static_cast<unsigned char>((1.0 - alpha) * curr_b + alpha * rgb_b);

	// Combine the components into an unsigned int color value
	unsigned int new_color = (new_r << 16) | (new_g << 8) | new_b;

	return new_color;
}

//Function used to draw pixels
void Draw_Pixel(ICBYTES& img, int x, int y, int color, double alpha = 1.0)
{
	//Get the color of the pixel before painting
	int curr_color = img.sU(static_cast<long long>(x), static_cast<long long>(y));
	//Combine it with the color we will paint (if there is no transparency, it becomes the color we will paint directly)
	img.sU(static_cast<long long>(x), static_cast<long long>(y)) = Create_New_Color(curr_color, color, alpha);
}

//Çizgi çizdirmek için kullanılan fonksiyon
void Draw_Line(ICBYTES& img, int x1, int y1, int x2, int y2, int color, double alpha = 1.0) {
	// Calculate the differences in x and y coordinates
	int dx = x2 - x1;
	int dy = y2 - y1;

	// Determine the number of steps (pixels) needed
	int steps = abs(dx) > abs(dy) ? abs(dx) : abs(dy);

	// Calculate the increments for each step
	float xIncrement = (float)dx / steps;
	float yIncrement = (float)dy / steps;

	// Initialize starting point
	float x = x1;
	float y = y1;

	// Draw each pixel along the line
	for (int i = 0; i <= steps; ++i) {
		Draw_Pixel(img, (int)x, (int)y, color, alpha);
		x += xIncrement;
		y += yIncrement;
	}
}

//Function used to fill a rectangle
void Fill_Rectangle(ICBYTES& img, int x, int y, int w, int h, int color, double alpha = 1.0)
{
	//Draw the rectangle by placing pixels along its area.
	for (int i = 0; i < w; i++)
	{
		for (int j = 0; j < h; j++)
		{
			Draw_Pixel(img, x + i, y + j, color, alpha);
		}
	}
}

//Function used to draw polygons
void Draw_Polygon(ICBYTES& img, ICBYTES& points, int color, double alpha = 1.0)
{
	//Learn how many points the /points matrix contains, that is, the number of columns.
	int points_count = points.Y();
	for (int i = 1; i < points_count; i++)
	{
		/*
		In the field number 1 of the matrix in the X dim, there is the x value of the position of the point,
		 and in field number 2 there is the y value of the position of the point.
		*/
		
		//The reason of i + 1 is that we also send the values of the next point for the line.
		Draw_Line(img, points.I(1, i), points.I(2, i), points.I(1, i + 1), points.I(2, i + 1), color, alpha);
		//Used to make small rectangular marks in the corners
		Fill_Rectangle(img, points.I(1, i) - 3, points.I(2, i) - 3, 7, 7, color, alpha);
	}
}

//Function used to fill polygons (using Scan Line Algorithm)
void Fill_Polygon(ICBYTES& img, ICBYTES& points, int color, double alpha = 1.0)
{
	int points_count = points.Y();
	int minY = INT_MAX;
	int maxY = INT_MIN;

	// Find minimum and maximum y-coordinates of the polygon
	for (int i = 1; i < points_count; i++)
	{
		minY = (minY < points.I(2, i)) ? minY : points.I(2, i);
		maxY = (maxY > points.I(2, i)) ? maxY : points.I(2, i);
	}

	// Iterate through each scan line
	for (int y = minY; y <= maxY; y++)
	{
		std::vector<int> intersections;

		// Find intersection points of the scan line with polygon edges
		for (int i = 1; i < points_count; i++)
		{
			int x1 = points.I(1, i);
			int y1 = points.I(2, i);
			int x2 = points.I(1, i + 1);
			int y2 = points.I(2, i + 1);

			// Check if the scan line intersects the edge
			if ((y1 <= y && y < y2) || (y2 <= y && y < y1))
			{
				int x = x1 + (y - y1) * (x2 - x1) / (y2 - y1);
				intersections.push_back(x);
			}
		}

		// Sort the intersection points
		std::sort(intersections.begin(), intersections.end());

		// Fill pixels between pairs of intersection points
		for (size_t i = 0; i < intersections.size(); i += 2)
		{
			int x1 = intersections[i];
			int x2 = intersections[i + 1];
			Draw_Line(img, x1, y, x2, y, color, alpha);
		}
	}
}

//A different implementation (For Learning Purposes)
#pragma region The Nonzero Winding Number Rule 
int CalculateWindingNumber(int x, int y, ICBYTES& points)
{
	int windingNumber = 0;
	int pointsCount = points.Y();

	for (int i = 1; i < pointsCount; i++)
	{
		int x1 = points.I(1, i);
		int y1 = points.I(2, i);
		int x2 = points.I(1, i + 1);
		int y2 = points.I(2, i + 1);

		if (y1 <= y)
		{
			if (y2 > y && ((x2 - x1) * (y - y1) - (x - x1) * (y2 - y1)) > 0)
			{
				windingNumber++;
			}
		}
		else
		{
			if (y2 <= y && ((x2 - x1) * (y - y1) - (x - x1) * (y2 - y1)) < 0)
			{
				windingNumber--;
			}
		}
	}

	return windingNumber;
}
void Fill_Polygon_Nonzero_Winding_Number_Rule(ICBYTES& img, ICBYTES& points, int color, double alpha = 1.0)
{
	int minY = INT_MAX;
	int maxY = INT_MIN;
	int minX = INT_MAX;
	int maxX = INT_MIN;

	// Find minimum and maximum y-coordinates of the polygon
	int pointsCount = points.Y();
	for (int i = 1; i < pointsCount; i++)
	{
		minY = (minY < points.I(2, i)) ? minY : points.I(2, i);
		maxY = (maxY > points.I(2, i)) ? maxY : points.I(2, i);
		minX = (minX < points.I(1, i)) ? minX : points.I(1, i);
		maxX = (maxX > points.I(1, i)) ? maxX : points.I(1, i);
	}

	// Iterate through each scan line
	for (int y = minY; y <= maxY; y++)
	{
		// Iterate through each pixel on the scan line
		for (int x = minX; x <= maxX; x++)
		{
			// Calculate the winding number for the current pixel
			int windingNumber = CalculateWindingNumber(x, y, points);

			// If the winding number is nonzero, the pixel is inside the polygon
			if (windingNumber != 0)
			{
				Draw_Pixel(img, x, y, color, alpha);
			}
		}
	}
}
#pragma endregion

// Function to draw the legs (lines connecting data points to the center)
void Draw_Spider_Chart_Core(ICBYTES& img, int x, int y, int r, int category, int value, int color) {
	for (int i = 0; i < category; ++i)
	{
		/*int slide_angle;
		if (category % 2 != 0)
			slide_angle = 90;
		else
		{
			int number = category;
			int counter = 0;
			for (int i = 0; i < 2; i++)
			{
				if (number % 2 == 0)
				{
					number = number / 2;
					counter++;
				}
			}
			slide_angle = counter < 2 ? - 360 / category : - 180 / category;
		}*/
		//The upper part was necessary to ensure that the central leg was on top, not below, but we did not use it, but since it might be necessary, we put it in the comments line.

		// Calculate the angle for each data point
		double curr_angle = ((360.0 * i / category) + 90/*slide_angle*/) * M_PI / 180.0;

		// Calculate the (x, y) coordinates
		int curr_leg_x = x + (int)(r * cos(curr_angle));
		int curr_leg_y = y + (int)(r * sin(curr_angle));

		// Draw a line from the center to (x, y)
		Draw_Line(img, x, y, curr_leg_x, curr_leg_y, color);

		// Calculate the next angle for each data point
		double next_angle = ((360.0 * (i + 1) / category) + 90/*slide_angle*/) * M_PI / 180.0;

		if (value > 0) {
			// Calculate draw lines
			for (int i = 0; i < value + 1; ++i)
			{
				curr_leg_x = x + (int)(r / value * i * cos(curr_angle));
				curr_leg_y = y + (int)(r / value * i * sin(curr_angle));
				int next_leg_x = x + (int)(r / value * i * cos(next_angle));
				int next_leg_y = y + (int)(r / value * i * sin(next_angle));

				Draw_Line(img, curr_leg_x, curr_leg_y, next_leg_x, next_leg_y, color);
			}
		}
	}
}

//Function that displays the numeric data values of the relevant data object
void Show_Spider_Chart_Values(ICBYTES& img, ICBYTES& points, int id, int color, double alpha = 1.0)
{
	int points_count = points.Y();

	for (int i = 1; i < points_count; i++)
	{
		//Inserting the data value into a string by taking the 2 digits after ",".
		std::ostringstream myStream;
		myStream << std::fixed << std::setprecision(2);
		myStream << main_data.D(i, id);
		std::string str_data = myStream.str();
		const char* c = str_data.c_str();

		//Numbers are written in line with all the points in order.
		Impress12x20(img, points.I(1, i), points.I(2, i), c, Create_New_Color(theme_color == 0 ? 0xffffff : 0, color, 0.7));
	}
}

//Function that draws the data objects of the Spider Chart as polygons
void Draw_Spider_Chart(ICBYTES& img, ICBYTES& data, int x, int y, int r, int color, double alpha = 1)
{
	int category_number = data.X();
	int sample_number = data.Y();
	const int value_index = 5;
	if (sample_number > 0)
	{
		//Loop to show each data
		for (int i = 1; i < sample_number + 1; i++)
		{
			int data_id = data_order.I(i);

			//If this data sample is hidden by clicking the button, it moves to the next element of the loop.
			if (data_visibility.I(data_id) % 2 == 0) continue;//Move to the next element and skip the code below.
			
			//Create matrix to calculate and keep points of data to be displayed
			ICBYTES points(__LINE__);
			CreateMatrix(points, 2, category_number + 1, ICB_INT);

			//Calculate current angle for placing points
			double curr_angle = ((360.0 / category_number) + 90) * M_PI / 180.0;
			//According to curr_angle, define point_x and point_y as the first element.
			int point_x = x + (int)(r / value_index * data.D(1, data_id) * cos(curr_angle));
			int point_y = y + (int)(r / value_index * data.D(1, data_id) * sin(curr_angle));

			//Define these values ​​to the last element. Because the first and last element are the same (polygon)
			points.I(1, category_number + 1) = point_x;
			points.I(2, category_number + 1) = point_y;

			//Calculate the values ​​of all other points and add them to the matrix
			for (int c = 1; c < category_number + 1; c++)
			{
				curr_angle = ((360.0 * c / category_number) + 90) * M_PI / 180.0;
				point_x = x + (int)(r / value_index * data.D(c, data_id) * cos(curr_angle));
				point_y = y + (int)(r / value_index * data.D(c, data_id) * sin(curr_angle));
				points.I(1, c) = point_x;
				points.I(2, c) = point_y;
			}

			int data_color;
			//Set color of data to draw
			switch (data_id)
			{
			case 1:
				data_color = 0x3c59fc;
				break;
			case 2:
				data_color = 0xff5631;
				break;
			case 3:
				data_color = 0xc5e626;
				break;
			case 4:
				data_color = 0xffcd00;
				break;
			case 5:
				data_color = 0x00feb0;
				break;
			case 6:
				data_color = 0xff2de0;
				break;
			default:
				break;
			}
			//Fill the points of the polygon with the specified color (and alpha)
			Fill_Polygon(img, points, data_color, alpha);
			//Draw the edges of the polygon with non-transparent color and the points at the corners
			Draw_Polygon(image, points, data_color, 0.9); 

			////If numeric data values are to be displayed, prints that values
			if (value_visibility.I(data_id) % 2 == 1) Show_Spider_Chart_Values(image, points, data_id, data_color, 0.9);
			
			Free(points);//Free is the method used to clear the memory area when work is done with the ICBYTES object.
		}
	}
}

//Function that draws the Spider Chart by calling other functions after scaling the data
void Spider_Chart(ICBYTES& img, ICBYTES& data, int x, int y, int r, int color, double alpha = 1, bool new_data = true)
{
	
	int category_number = data.X();
	int sample_number = data.Y();
	const int value_index = 5;
	//If the chart is drawn with different values for the first time, reset the visibility and order parameters of the chart
	if (new_data)
	{
		Free(data_visibility);
		CreateMatrix(data_visibility, sample_number, ICB_INT);
		data_visibility = 1;
		Free(value_visibility);
		CreateMatrix(value_visibility, sample_number, ICB_INT);
		value_visibility = 0;
		Free(data_order);
		CreateMatrix(data_order, sample_number, ICB_INT);
		for (int i = 1; i < sample_number + 1; i++)
			data_order.I(i) = i;
	}
	ICBYTES scaled_data(__LINE__);//Create matrix to scale data
	CreateMatrix(scaled_data, category_number, sample_number, ICB_DOUBLE);
	scaled_data = 0.0;

	ICBYTES max_numbers_and_divisors(__LINE__);
	/*
	Create a matrix to hold the number that is divisible by maximum 5 and its divisor.
	In the field number 1 of the matrix in the X dim, 
	there is the maxnum value of the position of the point, 
	and in field number 2 there is the divisor value of the position of the point.
	*/
	CreateMatrix(max_numbers_and_divisors, category_number, 2, ICB_DOUBLE);
	max_numbers_and_divisors = 0.0;

	for (int x = 1; x < category_number + 1; x++)//For each category
	{
		for (int y = 1; y < sample_number + 1; y++)
		{
			//Find the maximum number
			if (max_numbers_and_divisors.D(x, 1) < data.D(x, y))
				max_numbers_and_divisors.D(x, 1) = data.D(x, y);
		}

		//Round the maximum number to the ceiling number divisible by 5
		if(fmod(max_numbers_and_divisors.D(x, 1), 5.0) != 0.0)
			max_numbers_and_divisors.D(x, 1) = max_numbers_and_divisors.D(x, 1) + 5.0 - fmod(max_numbers_and_divisors.D(x, 1), 5.0);

		//Find and Set divisor number
		max_numbers_and_divisors.D(x, 2) = max_numbers_and_divisors.D(x, 1) / 5.0;

		//Scale the data to the range 0-5 by dividing the data into divisor.
		for (int y = 1; y < sample_number + 1; y++)
			scaled_data.D(x, y) = data.D(x, y) / max_numbers_and_divisors.D(x, 2); 
	}

	//Draw the core of the graph, that is, the gray lines, with the scaled data
	Draw_Spider_Chart_Core(img, x, y, r, category_number, value_index, color);
	//Draw data polygons on the graphics core
	Draw_Spider_Chart(img, scaled_data, x, y, r, color, alpha); 

	//Free allocated memory
	Free(max_numbers_and_divisors);
	Free(scaled_data);
}

//Function that allows to create random data
void RandomDataFill(ICBYTES& data, int category, int sample, int min, int max)
{
	Reset(data);
	if (sample <= 0 || category <= 0 || min < 0 || max < 0 || max < min)
		return;
	CreateMatrix(data, category, sample, ICB_DOUBLE);
	if (min == max)
	{
		//For efficiency, if these 2 values are the same, we exit by assigning the same value to the entire matrix.
		data = min;
		return;
	}
	std::random_device dev;
	std::mt19937 rng(dev());
	std::uniform_real_distribution<double> dist(min, max);

	for (int j = 1; j < sample + 1; j++)
	{
		for (int i = 1; i < category + 1; i++)
		{
			//Random value is generated within the min max range given to each data point
			data.D(i, j) = dist(rng);
		}
	}
}

//Function that converts the text in the TextBox into numerical data
int GetCategory()
{
	ICBYTES charsarr(__LINE__);
	GetText(SLE, charsarr);//The method we use to get the String in the TextBox
	int length = charsarr.X() - 1;
	int category = 0;
	for (int i = length; i > 0; i--)
	{
		char c = charsarr.C(i);
		//We check whether the chars are numeric values between 0-9.
		if (c < '0' || c > '9')
		{
			//If the invalid category is entered main_data is freed.
			Free(main_data); 
			category = 0;
			image = theme_color;
			//If they are not numeric, display an error message on the screen
			Impress12x20(image, 46, 245, "Please Enter Valid Category Number", theme_color == 0 ? 0xffffff : 0);
			DisplayImage(FRM, image);
			break;
		}
		category += (int)(c - '0') * pow(10, length - i);
	}
	//Free allocated memory
	Free(charsarr);
	return category;
}

//Function to create a chart with N samples
void CreateNSampleChart(int n)
{
	image = theme_color;//Clear the screen

	//Create random data with n number of samples in the range 0-5, with the value written in the Category box
	RandomDataFill(main_data, GetCategory(), n, 0, 5);

	Spider_Chart(image, main_data, 250, 250, 200, theme_color == 0 ? 0x444444 : 0xBBBBBB, 0.5);//Bu veriyi chart olarak göster
	DisplayImage(FRM, image);//Display the image drawn on the screen
}

//Function used to plot the current version of the chart when a change is made on the chart (such as changing the order, hiding the value, hiding the data).
void RefreshChart()
{
	image = theme_color;//Clear the screen
	Spider_Chart(image, main_data, 250, 250, 200, theme_color == 0 ? 0x444444 : 0xBBBBBB, 0.5, false);//Display the changed content in the chart.
	DisplayImage(FRM, image);//Display the image drawn on the screen
}

//Function that brings the relevant data to the front when any function button is pressed
void BringDataToFront(int id)
{
	int id_index = 0;
	int data_size = data_order.X();

	//Find the location of the relevant data
	for (int i = 1; i < data_size + 1; i++)
		if (data_order.I(i) == id) id_index = i;

	//Shift all data in front of the relevant data back by 1 unit
	for (int i = id_index; i < data_size; i++)
		data_order.I(i) = data_order.I(i + 1);

	//Put the relevant data into the space opened in front of the row
	data_order.I(data_size) = id;
}

//Function of the hide and show buttons
void ButtonEvents(void* p)
{
	int id = *(int*)p;
	int sample = main_data.Y();
	//To prevent the following code from being executed for functions with id+1 parameter larger than the number of samples.
	if (id + 1 > sample) return;

	//Increase visibility and change the color of the tile according to its final state
	if (++data_visibility.I(id+1) % 2 == 0)
	{
		Buttons[id].ButtonImage = theme_color;
		Fill_Rectangle(Buttons[id].ButtonImage, 9, 9, 4, 4, Buttons[id].ButtonColor);
	}
	else
	{
		Buttons[id].ButtonImage = Buttons[id].ButtonColor;

		//If it is visible, bring the relevant data to the front
		BringDataToFront(id+1);
	}
	SetButtonBitmap(Buttons[id].ButtonHandle, Buttons[id].ButtonImage);

	//Redraw data whose visibility values have changed.
	RefreshChart();
}

//Function performed by buttons that hide and show values
void ValueButtonEvents(void* p)
{
	int id = *(int*)p;
	int sample = main_data.Y();
	//To prevent the following code from being executed for functions with id+1 parameter larger than the number of samples.
	if (id + 1 > sample) return;

	ICBYTES buttontext(__LINE__);
	CreateMatrix(buttontext, 5, ICB_CHAR);
	//Increases the visibility value and changes the text of the box according to its final status
	if (++value_visibility.I(id + 1) % 2 == 0)
	{
		buttontext.C(1) = 'S';
		buttontext.C(2) = 'h';
		buttontext.C(3) = 'o';
		buttontext.C(4) = 'w';
		buttontext.C(5) = '\0';
	}
	else
	{
		buttontext.C(1) = 'H';
		buttontext.C(2) = 'i';
		buttontext.C(3) = 'd';
		buttontext.C(4) = 'e';
		buttontext.C(5) = '\0';

		//If its value is visible, bring the relevant data to the front.
		BringDataToFront(id + 1);
	}
	SetText(Buttons[id].ValueButtonHandle, buttontext);
	//Free allocated memory
	Free(buttontext);

	//Redraw data whose visibility values have changed.
	RefreshChart();
}

//Function that determines the appearance of buttons
void RefreshButtons(int n)
{
	ICBYTES buttontext(__LINE__);
	CreateMatrix(buttontext, 5, ICB_CHAR);
	for (int i = 0; i < n; i++)//Refresh n Buttons
	{
		if (data_visibility.I(i + 1) % 2 == 0)
		{
			Buttons[i].ButtonImage = theme_color;
			Fill_Rectangle(Buttons[i].ButtonImage, 9, 9, 4, 4, Buttons[i].ButtonColor);
		}
		else
		{
			Buttons[i].ButtonImage = Buttons[i].ButtonColor;
		}
		SetButtonBitmap(Buttons[i].ButtonHandle, Buttons[i].ButtonImage);

		if (value_visibility.I(i + 1) % 2 == 0)
		{
			buttontext.C(1) = 'S';
			buttontext.C(2) = 'h';
			buttontext.C(3) = 'o';
			buttontext.C(4) = 'w';
			buttontext.C(5) = '\0';
		}
		else
		{
			buttontext.C(1) = 'H';
			buttontext.C(2) = 'i';
			buttontext.C(3) = 'd';
			buttontext.C(4) = 'e';
			buttontext.C(5) = '\0';
		}
		SetText(Buttons[i].ValueButtonHandle, buttontext);
	}
	buttontext.C(1) = '\0';//Empty text
	for (int i = n; i < 6; i++)//Clear properties of other Buttons
	{
		Buttons[i].ButtonImage = theme_color;//Empty color (background color)
		SetButtonBitmap(Buttons[i].ButtonHandle, Buttons[i].ButtonImage);
		SetText(Buttons[i].ValueButtonHandle, buttontext);
	}
	//Free allocated memory
	Free(buttontext);
}

//Function that calls functions that create samples in the desired number of samples, draw them, and change the appearance of the buttons.
void ButtonCharts(void* p)
{
	int sample = *(int*)p;
	if (GetCategory() != 0)//If the category is valid
	{
		CreateNSampleChart(sample);//Draw the graph
		RefreshButtons(sample);//Reset the states of the buttons
	}
	else
	{
		//Reset the states of the buttons (when called with 0, it makes all buttons appear empty)
		RefreshButtons(0);
	}
}

//Function that changes the theme to dark or light
void ChangeTheme()
{
	//Change button color according to theme color
	theme_color = theme_color == 0 ? 0xffffff : 0;
	theme_button_image = theme_color;
	SetButtonBitmap(TBTN, theme_button_image);//Assign this color to the button
	int sample = main_data.Y();
	if (sample > 0)
	{
		RefreshChart();//Re-draw the chart with renewed colors
	}
	else //If main_data was emptied because an invalid category was entered
	{
		//Display Error Message
		image = theme_color;
		Impress12x20(image, 46, 245, "Please Enter Valid Category Number", theme_color == 0 ? 0xffffff : 0);
		DisplayImage(FRM, image);
	}
	//Reset the states of the buttons
	RefreshButtons(sample);
}

//Main Function
void ICGUI_main()
{
	/*MLE = ICG_MLEditSunken(650, 5, 300, 550, "", SCROLLBAR_V);
	diag.SetOutput(ICG_GetHWND(MLE));*/
	ICG_Static(6, 7, 70, 20, "Category:"); //Category Label
	SLE = ICG_SLEditBorder(70, 6, 30, 20, "3"); //Category Text Box (Default Value = 3)
	const static  int params[5] = { 2,3,4,5,6 }; //Parameters for send to functions parameter.
	//5 different sample buttons with own parameters
	ICG_Button(105, 6, 80, 20, "2-Sample", ButtonCharts, (void*)&params[0]);
	ICG_Button(190, 6, 80, 20, "3-Sample", ButtonCharts, (void*)&params[1]);
	ICG_Button(275, 6, 80, 20, "4-Sample", ButtonCharts, (void*)&params[2]);
	ICG_Button(360, 6, 80, 20, "5-Sample", ButtonCharts, (void*)&params[3]);
	ICG_Button(445, 6, 80, 20, "6-Sample", ButtonCharts, (void*)&params[4]);

	//Define Button structs with own id,image and colors
	for (int i = 0; i < 6; i++)
	{
		switch (i)
		{
		case 0:
			Buttons[i].ButtonColor = 0x3c59fc;
			break;
		case 1:
			Buttons[i].ButtonColor = 0xff5631;
			break;
		case 2:
			Buttons[i].ButtonColor = 0xc5e626;
			break;
		case 3:
			Buttons[i].ButtonColor = 0xffcd00;
			break;
		case 4:
			Buttons[i].ButtonColor = 0x00feb0;
			break;
		case 5:
			Buttons[i].ButtonColor = 0xff2de0;
			break;
		}
		Buttons[i].ButtonId = i;
		CreateImage(Buttons[i].ButtonImage, 20, 20, ICB_UINT);
	}

	for (int i = 0; i < 6; i++)//Create buttons
	{
		Buttons[i].ButtonImage = Buttons[i].ButtonColor;
		Buttons[i].ButtonHandle = ICG_BitmapButton(530, 530 + (i * 30) - (5 * 30), 20, 20, ButtonEvents, (void*)&Buttons[i].ButtonId);//Assign id parameters so that their functions are different
		SetButtonBitmap(Buttons[i].ButtonHandle, Buttons[i].ButtonImage);
		Buttons[i].ValueButtonHandle = ICG_Button(555, 530 + (i * 30) - (5 * 30), 90, 20, "Show", ValueButtonEvents, (void*)&Buttons[i].ButtonId);//Assign id parameters so that their functions are different
	}
	

	ICG_Static(575, 8, 60, 20, "Theme:"); //Theme Label
	TBTN = ICG_BitmapButton(625, 6, 20, 20, ChangeTheme); //Theme Change Button
	CreateImage(theme_button_image, 20, 20, ICB_UINT); 
	theme_color = theme_color == 0 ? 0xffffff : 0;
	theme_button_image = theme_color;
	SetButtonBitmap(TBTN, theme_button_image); //Color for Theme Change Button

	CreateImage(image, 510, 510, ICB_UINT); //Create Main Image for Display Charts
	FRM = ICG_FrameMedium(5, 30, 520, 520); //Create Main Frame for Display Image
	RefreshButtons(0);
}