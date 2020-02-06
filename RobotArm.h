#include "include/Angel.h"
#include <cstdlib>
#include <iostream>


typedef Angel::vec4 point4;
typedef Angel::vec4 color4;




//----------------------------------------------------------------------------



class RobotArm
{
		private:
				vec3 position;
				mat4 robotModelViewProjection;

				int Index;
				
		public:

			void quad( int a, int b, int c, int d );
			void colorcube();
			void base(mat4 PVT);
			void upper_arm(mat4 PVT);
			void lower_arm(mat4 PVT);
			vec2 getTipPosition();
			void init(void);
			void displayRobotArm(mat4 Projection, mat4 View);

			vec3 getPos();
			void setTheta(int Arm, int value);
			void incTheta(int Arm, int value);

};