#include "RobotArm.h"
#include "include/Angel.h"
#include <cstdlib>
#include <iostream>

const int NumVertices = 36; //(6 faces)(2 triangles/face)(3 vertices/triangle)
extern GLuint vPosition, vColor;
point4 points[NumVertices];
color4 colors[NumVertices];

point4 vertices[8] = 
{
    point4( -0.5, -0.5,  0.5, 1.0 ),
    point4( -0.5,  0.5,  0.5, 1.0 ),
    point4(  0.5,  0.5,  0.5, 1.0 ),
    point4(  0.5, -0.5,  0.5, 1.0 ),
    point4( -0.5, -0.5, -0.5, 1.0 ),
    point4( -0.5,  0.5, -0.5, 1.0 ),
    point4(  0.5,  0.5, -0.5, 1.0 ),
    point4(  0.5, -0.5, -0.5, 1.0 )
};

// RGBA olors
color4 vertex_colors[8] = 
{

    color4( 0.9, 0.91, 0.98, 1.0 ),  // silver
    color4( 0.9, 0.91, 0.98, 1.0 ),  // silver
    color4( 0.9, 0.91, 0.98, 1.0 ),  // silver
    color4( 0.9, 0.91, 0.98, 1.0 ),  // silver
    color4( 0.9, 0.91, 0.98, 1.0 ),  // silver
    color4( 0.9, 0.91, 0.98, 1.0 ),  // silver
    color4( 0.9, 0.91, 0.98, 1.0 ),  // silver
    color4( 0.9, 0.91, 0.98, 1.0 )   // silver
};


// Parameters controlling the size of the Robot's arm
const GLfloat BASE_HEIGHT      = 2.0;
const GLfloat BASE_WIDTH       = 5.0;
const GLfloat LOWER_ARM_HEIGHT = 11.0;
const GLfloat LOWER_ARM_WIDTH  = 0.5;
const GLfloat UPPER_ARM_HEIGHT = 11.0;
const GLfloat UPPER_ARM_WIDTH  = 0.5;

// const GLfloat BASE_HEIGHT      = 2.0;
// const GLfloat BASE_WIDTH       = 5.0;
// const GLfloat LOWER_ARM_HEIGHT = 12.0;
// const GLfloat LOWER_ARM_WIDTH  = 0.5;
// const GLfloat UPPER_ARM_HEIGHT = 11.0;
// const GLfloat UPPER_ARM_WIDTH  = 0.5;
// Shader transformation matrices
mat4  model_view;
GLuint locModelViewProjection;
GLuint vao, vbo;

// Array of rotation angles (in degrees) for each rotation axis
enum { Base = 0, LowerArm = 1, UpperArm = 2, NumAngles = 3 };
int      Axis = Base;


// Menu option values
const int  Quit = 4;

GLfloat  Theta[NumAngles] = { 0.0 };
void RobotArm::quad( int a, int b, int c, int d )
{
    colors[Index] = vertex_colors[a]; points[Index] = vertices[a]; Index++;
    colors[Index] = vertex_colors[a]; points[Index] = vertices[b]; Index++;
    colors[Index] = vertex_colors[a]; points[Index] = vertices[c]; Index++;
    colors[Index] = vertex_colors[a]; points[Index] = vertices[a]; Index++;
    colors[Index] = vertex_colors[a]; points[Index] = vertices[c]; Index++;
    colors[Index] = vertex_colors[a]; points[Index] = vertices[d]; Index++;
}


void RobotArm::colorcube()
{
    quad( 1, 0, 3, 2 );
    quad( 2, 3, 7, 6 );
    quad( 3, 0, 4, 7 );
    quad( 6, 5, 1, 2 );
    quad( 4, 5, 6, 7 );
    quad( 5, 4, 0, 1 );
}

void RobotArm::base(mat4 PVT) //(const mat4 &vp)
{
    mat4 instance = ( Translate( 0.0, 0.5 * BASE_HEIGHT, 0.0 ) *Scale( BASE_WIDTH, BASE_HEIGHT, BASE_WIDTH ) );

    glUniformMatrix4fv( locModelViewProjection, 1, GL_TRUE, PVT * robotModelViewProjection * instance );

    glDrawArrays( GL_TRIANGLES, 0, NumVertices );
}

void RobotArm::upper_arm(mat4 PVT) 
{
    mat4 instance = ( Translate( 0.0, 0.5 * UPPER_ARM_HEIGHT, 0.0 ) *
                        Scale( UPPER_ARM_WIDTH, UPPER_ARM_HEIGHT, UPPER_ARM_WIDTH ) );
    
    glUniformMatrix4fv( locModelViewProjection, 1, GL_TRUE, PVT * robotModelViewProjection * instance);
    glDrawArrays( GL_TRIANGLES, 0, NumVertices );
}

void RobotArm::lower_arm(mat4 PVT)
{
    mat4 instance = ( Translate( 0.0, 0.5 * LOWER_ARM_HEIGHT, 0.0 ) * 
                        Scale( LOWER_ARM_WIDTH, LOWER_ARM_HEIGHT, LOWER_ARM_WIDTH ) );
    
    glUniformMatrix4fv(locModelViewProjection, 1, GL_TRUE, PVT * robotModelViewProjection * instance);
    glDrawArrays( GL_TRIANGLES, 0, NumVertices );
}

vec2 RobotArm::getTipPosition()
{
    vec2 tip;
    // base
    tip.x += position.x/2;
    tip.y += position.y + BASE_HEIGHT;
    // lower arm
    tip.x += LOWER_ARM_HEIGHT * -sin(3.14159/180* Theta[LowerArm]);
    tip.y += LOWER_ARM_HEIGHT * cos(-3.14159/180* Theta[LowerArm]);
    // upper arm
    tip.x += (UPPER_ARM_HEIGHT-0.5) * -cos(3.14159/180* (90 - Theta[LowerArm] - Theta[UpperArm]));
    tip.y += (UPPER_ARM_HEIGHT-0.5) * sin(3.14159/180* (90 - Theta[LowerArm] - Theta[UpperArm]));
    // round
    tip.x = (int)(0.5 + tip.x);
    tip.y = (int)(0.5 + tip.y);
    
    return tip;
}


void RobotArm::init(void)
{
    position = vec3(-10,0,0);
    Index =0;
    colorcube();
    // Create a vertex array object
    glGenVertexArrays( 1, &vao );
    glBindVertexArray( vao );

    // Create and initialize a buffer object
    glGenBuffers( 1, &vbo );
    glBindBuffer( GL_ARRAY_BUFFER, vbo );
    glBufferData( GL_ARRAY_BUFFER, sizeof(points) + sizeof(colors), NULL, GL_DYNAMIC_DRAW );
    glBufferSubData( GL_ARRAY_BUFFER, 0, sizeof(points), points );
    glBufferSubData( GL_ARRAY_BUFFER, sizeof(points), sizeof(colors), colors );

    glEnableVertexAttribArray( vPosition );
    glVertexAttribPointer( vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0) );

    glEnableVertexAttribArray( vColor );
    glVertexAttribPointer( vColor, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(points)));
}

vec3 RobotArm::getPos()
{
    return position;
}

void RobotArm::displayRobotArm(mat4 Projection, mat4 View)
{
        glBindVertexArray(vao);
        //base
        mat4 PVT = Projection * View * Translate(position);
        robotModelViewProjection = RotateY(Theta[Base]);
        base(PVT);
        //lower arm
        robotModelViewProjection = robotModelViewProjection * Translate(0.0, BASE_HEIGHT, 0.0);
        robotModelViewProjection = robotModelViewProjection * RotateZ(Theta[LowerArm]);
        lower_arm(PVT);
        //upper_arm
        robotModelViewProjection = robotModelViewProjection * Translate(0.0, LOWER_ARM_HEIGHT, 0.0);
        robotModelViewProjection = robotModelViewProjection * RotateZ(Theta[UpperArm]);
        upper_arm(PVT);

        robotModelViewProjection = robotModelViewProjection * Translate(0.0, UPPER_ARM_HEIGHT, 0.0);

}

void RobotArm::setTheta(int Arm, int value)
{
    Theta[Arm] = value;
}

void RobotArm::incTheta(int Arm, int value)
{
    Theta[Arm] += value;
}

