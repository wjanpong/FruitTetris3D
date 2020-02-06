/*
CMPT 361 Assignment 1 - FruitTetris implementation Sample Skeleton Code

- This is ONLY a skeleton code showing:
How to use multiple buffers to store different objects
An efficient scheme to represent the grids and blocks

- Compile and Run:
Type make in terminal, then type ./FruitTetris

This code is extracted from Connor MacLeod's (crmacleo@sfu.ca) assignment submission
by Rui Ma (ruim@sfu.ca) on 2014-03-04. 

Modified in Sep 2014 by Honghua Li (honghual@sfu.ca).
*/

#include "include/Angel.h"
#include "RobotArm.h"
#include <cstdlib>
#include <iostream>
#include <set>
#include <vector>
#include <algorithm>
#include <sstream>


using namespace std;


int randomShape;//generate a random shape
int randomOritation;//generate a random oritation

// the index of current tile
int currTileIdx = 0;


RobotArm RA;//object to impelent Robot Arm

// xsize and ysize represent the window size - updated if window is reshaped to prevent stretching of the game
int xsize = 400; 
int ysize = 720;
int color[4]={5,5,5,5};

// current tile
vec2 tile[4]; // An array of 4 2d vectors representing displacement from a 'center' piece of the tile, on the grid
vec2 tilepos = vec2(5, 19); // The position of the current tile using grid coordinates ((0,0) is the bottom left corner)

// An array storing all possible orientations of all possible tiles
// The 'tile' array will always be some element [i][j] of this array (an array of vec2)
vec2 allRotationsLshape[4][4] = 
	{{vec2(-1,-1), vec2(-1,0), vec2(0, 0), vec2(1,0)},
	{vec2(1, -1), vec2(0, -1), vec2(0,0), vec2(0, 1)},     
	{vec2(1, 1), vec2(1,0), vec2(0, 0), vec2(-1, 0)},  
	{vec2(-1,1), vec2(0, 1), vec2(0, 0), vec2(0, -1)}};

vec2 allRotationsIshape[4][4] = 
	{{vec2(-2, 0), vec2(-1,0), vec2(0, 0), vec2(1,0)},
	{vec2(0, -2), vec2(0, -1), vec2(0,0), vec2(0, 1)},
	{vec2(-2, 0), vec2(-1,0), vec2(0, 0), vec2(1,0)},
	{vec2(0, -2), vec2(0, -1), vec2(0,0), vec2(0,1)}}; //for I

vec2 allRotationsSshape[4][4] = 
	{{vec2(-1, -1), vec2(0,-1), vec2(0, 0), vec2(1,0)},
	{vec2(1,-1), vec2(1, 0), vec2(0, 0),vec2(0, 1)},
	{vec2(-1, -1), vec2(0,-1), vec2(0, 0), vec2(1,0)},
	{vec2(1,-1), vec2(1, 0), vec2(0, 0),vec2(0, 1)}}; //for S

vec2 allRotationsTshape[4][4] = 
	{{vec2(-1,0), vec2(0, 0), vec2(1,0),vec2(0, -1)},
	{vec2(0, -1), vec2(0, 0), vec2(0,1), vec2(1, 0)},     
	{vec2(1, 0), vec2(0,0), vec2(-1, 0), vec2(0,  1)},  
	{vec2(0,1), vec2(0, 0), vec2(0,-1), vec2(-1, 0)}}; //for T

// colors
vec4 orange = vec4(1.0, 0.5, 0.0, 1.0); 
vec4 white  = vec4(1.0, 1.0, 1.0, 1.0);
vec4 black  = vec4(0.0, 0.0, 0.0, 0.0); 
vec4 boardColor = vec4(0.0, 0.0, 0.0, 0.0); 
vec4 grid   = vec4(0.658824, 0.658824, 0.658824, 0.7); 
vec4 grey   = vec4(0.658824, 0.658824, 0.658824, 1.0);
vec4 yellow = vec4(1.0, 1.0, 0.0, 1.0);
vec4 green  = vec4(0.5, 1.0, 0.5, 1.0);
vec4 purple = vec4(0.73, 0.16, 0.96, 1.0);
vec4 red  	= vec4(1.0, 0.0, 0.0, 1.0);
vec4 silver = vec4(0.9, 0.91, 0.98, 1.0);

//board[x][y] represents whether the cell (x,y) is occupied
bool board[10][20]; 
bool highlight = false;

//An array containing the colour of each of the 10*20*2*3*6 vertices that make up the board
//Initially, all will be set to black. As tiles are placed, sets of 6 vertices (2 triangles; 1 square)
//will be set to the appropriate colour in this array before updating the corresponding VBO
vec4 boardcolours[1200*6];
vec4 boardpoints[1200*6];

// location of vertex attributes in the shader program
GLuint vPosition;
GLuint vColor;

// locations of uniform variables in shader program
GLuint locxsize;
GLuint locysize;

// VAO and VBO
GLuint vaoIDs[3]; // One VAO for each object: the grid, the board, the current piece
GLuint vboIDs[6]; // Two Vertex Buffer Objects for each VAO (specifying vertex positions and colours, respectively)

bool isGameOver = false;
bool isRelease = false; //is 'space' is pressed

mat4 Projection,View, Model;
GLuint locM;

vec4 newcolours[24*6];//Tile color

float remainingTime = 5;//time remaining before the current tile is dropped

enum { Base = 0, LowerArm = 1, UpperArm = 2, NumAngles = 3 };

template<class T>
void writeText(T str, float x, float y) 
{
	stringstream remainingTimeText(str);
	glRasterPos2f(x, y);
	char c;
	while(remainingTimeText >> noskipws >> c) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c);
}

bool isOutBoardBounds(vec2 p) 
{
	if(p.x < 0 || p.x > 9) 
		return true;
	if(p.y < 0 || p.y > 19) 
		return true;
	return false;
}

void Collision()
{
	    highlight = false;
		if(!isRelease )//when the tile is at the tip && enable
		{
			tilepos = RA.getTipPosition();
			for (int i=0; i<4; i++)
			{

				vec2 p = tilepos + tile[i];

				if(isOutBoardBounds(p) || (board[GLuint(tile[i].x + tilepos.x)][GLuint(tile[i].y + tilepos.y)] == true))		
				{
						highlight = true;
						break;
				}
			}
			if (highlight)
			{
				vec4 highlightColor[24*6];
				for (int j=0; j<24*6; j++)
					highlightColor[j] = grey;

				glBindBuffer(GL_ARRAY_BUFFER, vboIDs[5]); // Bind the VBO containing current tile vertex colours
				glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(highlightColor), highlightColor); // Put the colour data in the VBO


			}
			else
			{
				glBindBuffer(GL_ARRAY_BUFFER, vboIDs[5]); // Bind the VBO containing current tile vertex colours
				glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(newcolours), newcolours); // Put the colour data in the VBO

			}
		}

}

// When the current tile is moved or rotated (or created), update the VBO containing its vertex position data
void updatetile()
{
	Collision();

	// Bind the VBO containing current tile vertex positions
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[4]); 

	// For each of the 4 'cells' of the tile,
	for (int i = 0; i < 4; i++) 
	{
		// Calculate the grid coordinates of the cell
		GLfloat x = tilepos.x + tile[i].x; 
		GLfloat y = tilepos.y + tile[i].y;

		// Create the 4 corners of the square - these vertices are using location in pixels
		// These vertices are later converted by the vertex shader
		vec4 p1 = vec4(33.0 + (x * 33.0), 33.0 + (y * 33.0), 16.50, 1); // front left bottom
		vec4 p2 = vec4(33.0 + (x * 33.0), 66.0 + (y * 33.0), 16.50, 1); // front left top
		vec4 p3 = vec4(66.0 + (x * 33.0), 33.0 + (y * 33.0), 16.50, 1); // front right bottom
		vec4 p4 = vec4(66.0 + (x * 33.0), 66.0 + (y * 33.0), 16.50, 1); // front right top
		vec4 p5 = vec4(33.0 + (x * 33.0), 33.0 + (y * 33.0), -16.50, 1); // back left bottom
		vec4 p6 = vec4(33.0 + (x * 33.0), 66.0 + (y * 33.0), -16.50, 1); // back left top
		vec4 p7 = vec4(66.0 + (x * 33.0), 33.0 + (y * 33.0), -16.50, 1); // back right bottom
		vec4 p8 = vec4(66.0 + (x * 33.0), 66.0 + (y * 33.0), -16.50, 1); // back right top

		// Two points are used by two triangles each
		vec4 newpoints[36] = {	p1, p2, p3, p2, p3, p4,
								p5, p6, p7, p6, p7, p8,
								p1, p2, p5, p2, p5, p6,
								p3, p4, p7, p4, p7, p8,
								p2, p4, p6, p4, p6, p8,
								p1, p3, p5, p3, p5, p7};

		// Put new data in the VBO
		glBufferSubData(GL_ARRAY_BUFFER, i*sizeof(newpoints), sizeof(newpoints), newpoints); 
	}

	//glBindVertexArray(0);
}

//-------------------------------------------------------------------------------------------------------------------
bool checkColor(int color[], int newColor) //give distinct color for 4 tiles
{
	for (int i=0; i<4;i++)
	{
		if(color[i]==newColor)
			return true;
	}
	return false;
}

// Called at the start of play and every time a tile is placed
void newtile()
{
	if(!isGameOver)
	{
			remainingTime = 5;
			isRelease = false;

			tilepos = RA.getTipPosition();

			randomShape = rand()%4;
			randomOritation = rand()%4;

			// Update the geometry VBO of current tile
			for (int i = 0; i < 4; i++)
			{
				switch(randomShape)
				{
					case 0: tile[i] = allRotationsLshape[randomOritation][i]; // Get the 4 pieces of the new tile
							break;   
					case 1: tile[i] = allRotationsIshape[randomOritation][i]; // Get the 4 pieces of the new tileI
							break;	
					case 2: tile[i] = allRotationsSshape[randomOritation][i]; // Get the 4 pieces of the new tileS
							break;
					case 3: tile[i] = allRotationsTshape[randomOritation][i]; // Get the 4 pieces of the new tileT
							break;	
					default:
							break;
				}
			}

			updatetile(); 

			for (int k=0;k<4;k++)
				color[k]=5;

			color[0]=rand()%5;
			int newColor;
			for(int k =1; k<4 ; k++)
			{
				do{
					newColor=rand()%5;
				}while(checkColor(color, newColor));
				color[k]=newColor;
			}

			for (int j = 0; j< 4; j++)
			{
				int currentColor = color[j];
				for (int i = j*36; i < j*36+36; i++)
				{
					switch(currentColor)
					{
						case 0: newcolours[i] = purple;
								break;
						case 1: newcolours[i] = red;
								break;
						case 2: newcolours[i] = yellow;
								break;
						case 3: newcolours[i] = green;
								break;
						case 4: newcolours[i] = orange;
								break;
						default: break;
					}
					
				}
			}



			glBindBuffer(GL_ARRAY_BUFFER, vboIDs[5]); // Bind the VBO containing current tile vertex colours
			glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(newcolours), newcolours); // Put the colour data in the VBO
			glBindBuffer(GL_ARRAY_BUFFER, 0);

			glBindVertexArray(0);
		}
}

//-------------------------------------------------------------------------------------------------------------------

void initGrid()
{
	// ***Generate geometry data
	vec4 gridpoints[64*2 + 462]; // Array containing the 64 points of the 32 total lines to be later put in the VBO, 462 for depth
	vec4 gridcolours[64*2 + 462]; // One colour per vertex

	// Vertical lines 
	for (int i = 0; i < 11; i++){
		gridpoints[2*i] = vec4((33.0 + (33.0 * i)), 33.0, 16.50, 1);
		gridpoints[2*i + 1] = vec4((33.0 + (33.0 * i)), 693.0, 16.50, 1);
		gridpoints[2*i + 64] = vec4((33.0 + (33.0 * i)), 33.0, -16.50, 1);
		gridpoints[2*i + 65] = vec4((33.0 + (33.0 * i)), 693.0, -16.50, 1);
		
	}
	// Horizontal lines
	for (int i = 0; i < 21; i++){
		gridpoints[22 + 2*i] = vec4(33.0, (33.0 + (33.0 * i)), 16.50, 1);
		gridpoints[22 + 2*i + 1] = vec4(363.0, (33.0 + (33.0 * i)), 16.50, 1);
		gridpoints[22 + 2*i + 64]	= vec4(33.0, (33.0 + (33.0 * i)), -16.50, 1);
		gridpoints[22 + 2*i + 65] 	= vec4(363.0, (33.0 + (33.0 * i)), -16.50, 1);
	}
	//Depth lines
	for (int i = 0; i < 21; i++)
	{
		for (int j = 0; j < 11; j++) 
		{
			gridpoints[128 + 22*i + 2*j] 		= vec4(33.0 + (j * 33.0), 33.0 + (i * 33.0), 16.50, 1); // front left bottom
			gridpoints[128 + 22*i + 2*j + 1] 	= vec4(33.0 + (j * 33.0), 33.0 + (i * 33.0), -16.50, 1); // back left bottom
		}
	}

	// Make all grid lines white
	for (int i = 0; i < 64*2 + 462; i++)
		gridcolours[i] = grid;


	// *** set up buffer objects
	// Set up first VAO (representing grid lines)
	glBindVertexArray(vaoIDs[0]); // Bind the first VAO
	glGenBuffers(2, vboIDs); // Create two Vertex Buffer Objects for this VAO (positions, colours)

	// Grid vertex positions
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[0]); // Bind the first grid VBO (vertex positions)
	glBufferData(GL_ARRAY_BUFFER, (64*2 + 462)*sizeof(vec4), gridpoints, GL_DYNAMIC_DRAW); // GL_STATIC_DRAW Put the grid points in the VBO
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, 0); 
	glEnableVertexAttribArray(vPosition); // Enable the attribute
	
	// Grid vertex colours
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[1]); // Bind the second grid VBO (vertex colours)
	glBufferData(GL_ARRAY_BUFFER, (64*2 + 462)*sizeof(vec4), gridcolours, GL_DYNAMIC_DRAW); // Put the grid colours in the VBO
	glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vColor); // Enable the attribute
}

void square(int offset, vec4 &p1, vec4 &p2, vec4 &p3, vec4 &p4, vec4 *boardpoints) 
{
	boardpoints[offset 	 ] = p1;
	boardpoints[offset + 1] = p2;
	boardpoints[offset + 2] = p3;
	boardpoints[offset + 3] = p2;
	boardpoints[offset + 4] = p3;
	boardpoints[offset + 5] = p4;
}

void colorSquare(int offset, vec4 &color)
{
	boardcolours[offset 	 ] = color;
	boardcolours[offset + 1] = color;
	boardcolours[offset + 2] = color;
	boardcolours[offset + 3] = color;
	boardcolours[offset + 4] = color;
	boardcolours[offset + 5] = color;
} 

void initBoard()
{
	// *** Generate the geometric data
	//vec4 boardpoints[1200*6];
	for (int i = 0; i < 1200*6; i++)
		boardcolours[i] = boardColor; // Let the empty cells on the board be black
	// Each cell is a square (2 triangles with 6 vertices)
	for (int i = 0; i < 20; i++){
		for (int j = 0; j < 10; j++)
		{		
			vec4 p1 = vec4(33.0 + (j * 33.0), 33.0 + (i * 33.0), 16.50, 1); // front left bottom
			vec4 p2 = vec4(33.0 + (j * 33.0), 66.0 + (i * 33.0), 16.50, 1); // front left top
			vec4 p3 = vec4(66.0 + (j * 33.0), 33.0 + (i * 33.0), 16.50, 1); // front right bottom
			vec4 p4 = vec4(66.0 + (j * 33.0), 66.0 + (i * 33.0), 16.50, 1); // front right top
			vec4 p5 = vec4(33.0 + (j * 33.0), 33.0 + (i * 33.0), -16.50, 1); // back left bottom
			vec4 p6 = vec4(33.0 + (j * 33.0), 66.0 + (i * 33.0), -16.50, 1); // back left top
			vec4 p7 = vec4(66.0 + (j * 33.0), 33.0 + (i * 33.0), -16.50, 1); // back right bottom
			vec4 p8 = vec4(66.0 + (j * 33.0), 66.0 + (i * 33.0), -16.50, 1); // back right top
			
			int offset = 36*(10*i + j);
			square(offset     , p1, p2, p3, p4, boardpoints); // front
			square(offset + 6 , p5, p6, p7, p8, boardpoints); // back
			square(offset + 12, p1, p2, p5, p6, boardpoints); // left
			square(offset + 18, p3, p4, p7, p8, boardpoints); // right
			square(offset + 24, p2, p4, p6, p8, boardpoints); // up
			square(offset + 30, p1, p3, p5, p7, boardpoints); // down
		}
	}

	// Initially no cell is occupied
	for (int i = 0; i < 10; i++)
		for (int j = 0; j < 20; j++)
			board[i][j] = false; 


	// *** set up buffer objects
	glBindVertexArray(vaoIDs[1]);
	glGenBuffers(2, &vboIDs[2]);

	// Grid cell vertex positions
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[2]);
	glBufferData(GL_ARRAY_BUFFER, 1200*6*sizeof(vec4), boardpoints, GL_STATIC_DRAW);
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vPosition);

	// Grid cell vertex colours
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[3]);
	glBufferData(GL_ARRAY_BUFFER, 1200*6*sizeof(vec4), boardcolours, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vColor);
}

// No geometry for current tile initially
void initCurrentTile()
{
	glBindVertexArray(vaoIDs[2]);
	glGenBuffers(2, &vboIDs[4]);

	// Current tile vertex positions
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[4]);
	glBufferData(GL_ARRAY_BUFFER, 24*6*sizeof(vec4), NULL, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vPosition);

	// Current tile vertex colours
	glBindBuffer(GL_ARRAY_BUFFER, vboIDs[5]);
	glBufferData(GL_ARRAY_BUFFER, 24*6*sizeof(vec4), NULL, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(vColor);
}

void init()
{
	// Load shaders and use the shader program
	GLuint program = InitShader("vshader.glsl", "fshader.glsl");
	glUseProgram(program);

	// Get the location of the attributes (for glVertexAttribPointer() calls)
	vPosition = glGetAttribLocation(program, "vPosition");
	vColor = glGetAttribLocation(program, "vColor");

	// Create 3 Vertex Array Objects, each representing one 'object'. Store the names in array vaoIDs
	glGenVertexArrays(3, &vaoIDs[0]);

	// Initialize the grid, the board, and the current tile
	initGrid();
	initBoard();
	initCurrentTile();
	RA.init();
	RA.setTheta(LowerArm, 5);
	RA.setTheta(UpperArm, -85);

	// The location of the uniform variables in the shader program
	locxsize = glGetUniformLocation(program, "xsize"); 
	locysize = glGetUniformLocation(program, "ysize");
	locM = glGetUniformLocation(program, "M");

	vec3 eye = vec3(0, 20 + 10, 24);
	vec3 look = vec3(0, 20/2, 0);
	vec3 up  = vec3(0, 1, 0);
	View = LookAt(eye,look,up);

	// Game initialization
	newtile(); // create new next tile

	// set to default
	glBindVertexArray(0);

	// Blend for transparent effect
   	glEnable(GL_BLEND); 
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glClearColor(0, 0, 0, 0);

	// Depth
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glClearDepth(1.0);
	// Antialiasing
	glEnable(GL_MULTISAMPLE);
	glHint(GL_MULTISAMPLE_FILTER_HINT_NV, GL_NICEST);
	glEnable(GL_LINE_SMOOTH);
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
	glEnable(GL_POINT_SMOOTH);
	glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
}

//-------------------------------------------------------------------------------------------------------------------

// Rotates the current tile, if there is room
void rotate()
{      

   bool flag = true;
	vec2 tempTile[4]; // temporary tile to check if the new oritation fit the window 
	randomOritation +=1;//rotate in counter clockwise direction
	if(randomOritation >=4)
		randomOritation =0;

	for (int i = 0; i < 4; i++)
	{
		switch(randomShape)
		{
			case 0: tempTile[i] = allRotationsLshape[randomOritation][i]; // Get the 4 pieces of the new tile
					break;
			case 1: tempTile[i] = allRotationsIshape[randomOritation][i]; // Get the 4 pieces of the new tileI
					break;	
			case 2: tempTile[i] = allRotationsSshape[randomOritation][i]; // Get the 4 pieces of the new tileS
					break;
			case 3: tempTile[i] = allRotationsTshape[randomOritation][i]; // Get the 4 pieces of the new tileT
					break;	
			default:
					break;
		}
	}
	for (int i = 0; i < 4; i++)
	{
		if((tempTile[i].x + tilepos.x <0) || (tempTile[i].x + tilepos.x >9) || (board[GLuint(tempTile[i].x + tilepos.x)][GLuint(tempTile[i].y + tilepos.y)] == true))
			flag = false;
	}
	if(flag) //if there is enough room for rotating
	{
		for (int i = 0; i < 4; i++)
		{
			tile[i] = tempTile[i];
		}
		updatetile();

	}
	
}

//-------------------------------------------------------------------------------------------------------------------

// Checks if the specified row (0 is the bottom 19 the top) is full
// If every cell in the row is occupied, it will clear that cell and everything above it will shift down one row
void checkfullrow(int row)
{

}

//-------------------------------------------------------------------------------------------------------------------

// Places the current tile - update the board vertex colour VBO and the array maintaining occupied cells
void settile()
{

			int x ;
			int y ;
			int minimumRow = 20; // minimum row of the current tile
			for (int k=0;k<4;k++)
			{	
				x = GLuint(tilepos.x + tile[k].x);
				y = GLuint(tilepos.y + tile[k].y);
				if(y<minimumRow)
					minimumRow=y;

				int offset = 36*(10*y + x);
				board[x][y]=true;
				// for (int i=0;i<36;i++)
				// 	boardcolours[offset +i]= newcolours[k*36];
				colorSquare(offset, newcolours[k*36]);
				colorSquare(offset + 6, newcolours[k*36]);
				colorSquare(offset + 12, newcolours[k*36]);
				colorSquare(offset + 18, newcolours[k*36]);
				colorSquare(offset +24, newcolours[k*36]);
				colorSquare(offset +30 , newcolours[k*36]);
			
			}


			// Grid cell vertex colours
			glBindBuffer(GL_ARRAY_BUFFER, vboIDs[3]);
			glBufferData(GL_ARRAY_BUFFER, 1200*6*sizeof(vec4), boardcolours, GL_DYNAMIC_DRAW);
			glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0, 0);
			glEnableVertexAttribArray(vColor);

			initGrid();

}

//-------------------------------------------------------------------------------------------------------------------

// Given (x,y), tries to move the tile x squares to the right and y squares down
// Returns true if the tile was successfully moved, or false if there was some issue
bool movetile(vec2 direction)
{
	for (int i=0;i<4;i++)
	{
		if ((tilepos.x + tile[i].x + direction.x <0 ) || (tilepos.x + tile[i].x + direction.x >9) || (tilepos.y +tile[i].y +direction.y <0 ) || (board[GLuint(tilepos.x + tile[i].x + direction.x)][GLuint(tilepos.y + tile[i].y + direction.y)] == true))
		{
			return false;
		}

	}
	tilepos.x += direction.x;
	tilepos.y += direction.y;
	return true;
}
//-------------------------------------------------------------------------------------------------------------------

// Starts the game over - empties the board, creates new tiles, resets line counters
void restart()
{
	isGameOver =false;
	init();
}
//-------------------------------------------------------------------------------------------------------------------
//-------------------------------------------------------------------------------------------------------------------
void move_down(int value)
{
	
	if(isRelease)
	{
		vec2 direction;
		direction.x=0;
		direction.y=-1;
		if(!movetile(direction))
		{
			//checkSameFruit();
			settile();
			newtile();
		}
		updatetile();
		glutTimerFunc(200,move_down,0);
	}
}
// Draws the game
void display()
{

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	Projection = Perspective(45, 1.0*xsize/ysize, 10, 200);
	//glUniform1i(locxsize, xsize); // x and y sizes are passed to the shader program to maintain shape of the vertices on screen
	//glUniform1i(locysize, ysize);
	RA.displayRobotArm(Projection, View);//Draw the robot Arm

    mat4 Model = mat4();
	Model *= Translate(0, 20/2.0, 0);
	Model *= Scale(1.0/33, 1.0/33, 1.0/33);  // scale to unit length
	Model *= Translate(-33*10/2.0 - 33, -33*20/2.0 - 33, 0); // move to origin

	mat4 M = Projection * View * Model;
	glUniformMatrix4fv(locM, 1, GL_TRUE, M);

    // glUniformMatrix4fv( model_view, 1, GL_TRUE, View );

    // mat4  p =  Perspective(45, 1.0*xsize/ysize, 10, 200);
    // glUniformMatrix4fv( projection, 1, GL_TRUE, p );

	glBindVertexArray(vaoIDs[1]); // Bind the VAO representing the grid cells (to be drawn first)
	glDrawArrays(GL_TRIANGLES, 0, 1200*6); // Draw the board (10*20*2 = 400 triangles)

	glBindVertexArray(vaoIDs[2]); // Bind the VAO representing the current tile (to be drawn on top of the board)
	glDrawArrays(GL_TRIANGLES, 0, 24*6); // Draw the current tile (8 triangles)

	glBindVertexArray(vaoIDs[0]); // Bind the VAO representing the grid lines (to be drawn on top of everything else)
	glDrawArrays(GL_LINES, 0, 64*2+462); // Draw the grid lines (21+11 = 32 lines)

	if(remainingTime >0)
		remainingTime = remainingTime - 1/60.0;
	else
	{ 

		remainingTime =0;
		if((!highlight) && (remainingTime ==0) && (!isGameOver))
		{		isRelease = true;
				move_down(0);
		}
		else if((highlight && (remainingTime ==0)) || (isGameOver))
		{
			stringstream gameOverText;
			gameOverText<<noskipws<<"Game Over!!! Press R to restart the Game";
			glColor4f(1.0f, 0.0f, 0.0f, 1.0f);
			writeText(gameOverText.str(), -1.0f , 0.4f);
			isGameOver =true;
		}
	}


	stringstream remainingTimeText;
	remainingTimeText<<noskipws<<"Remaining Time Before Tile Drops: "<<remainingTime;
	glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
	writeText(remainingTimeText.str(), -0.95 ,0.95);


	glutPostRedisplay();
	glutSwapBuffers();
}

//-------------------------------------------------------------------------------------------------------------------

// Reshape callback will simply change xsize and ysize variables, which are passed to the vertex shader
// to keep the game the same from stretching if the window is stretched
void reshape(GLsizei w, GLsizei h)
{
	xsize = w;
	ysize = h;
	glViewport(0, 0, w, h);
}

//-------------------------------------------------------------------------------------------------------------------

// Handle arrow key keypresses
void special(int key, int x, int y)
{

		switch(key) 
		{
				case GLUT_KEY_RIGHT:
					if(glutGetModifiers() == GLUT_ACTIVE_CTRL)
						View *= RotateY(5);
					break;
				case GLUT_KEY_LEFT:
					if(glutGetModifiers() == GLUT_ACTIVE_CTRL)
						View *= RotateY(-5);
					break;
				case GLUT_KEY_UP:
					if(glutGetModifiers() == GLUT_ACTIVE_CTRL)
						View *= RotateZ(5);
					else 
					{
						rotate();
					}
					break;
				case GLUT_KEY_DOWN:
					if(glutGetModifiers() == GLUT_ACTIVE_CTRL)
						View *= RotateZ(-5);
					break;

				default:
					break;
		}
}

// Handles standard keypresses
void keyboard(unsigned char key, int x, int y)
{
	switch(key) 
	{
		case 033: // Both escape key and 'q' cause the game to exit
		    exit(EXIT_SUCCESS);
		    break;
		case 'q':
			exit (EXIT_SUCCESS);
			break;
		case 'r': // 'r' key restarts the game
			restart();
			break;
		case 'a':
			RA.incTheta(LowerArm, 5);
			updatetile();
			break;
		case 'd':
			RA.incTheta(LowerArm, -5);
			updatetile();
			break;	
		case 'w':
			RA.incTheta(UpperArm, 5);
			updatetile();
			break;
		case 's':
			RA.incTheta(UpperArm, -5);
			updatetile();
			break;
		case ' ':
			if(!highlight)
			{
				isRelease=true;
				move_down(0);
			}
			break;
	}
	glutPostRedisplay();
}


//-------------------------------------------------------------------------------------------------------------------

void idle(void)
{
	glutPostRedisplay();
}


//-------------------------------------------------------------------------------------------------------------------

int main(int argc, char **argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_MULTISAMPLE | GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH );
	glutInitWindowSize(xsize, ysize);
	glutInitWindowPosition(680, 178); // Center the game window (well, on a 1920x1080 display)
	glutCreateWindow("Fruit Tetris");
	glewInit();
	init();

	// Callback functions
	//glutTimerFunc(300 , timerEvent, 0);
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutSpecialFunc(special);
	glutKeyboardFunc(keyboard);
	glutIdleFunc(idle);

	glutMainLoop(); // Start main loop
	return 0;
}
