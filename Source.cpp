/*

MIT License

Copyright (c) 2022 Tyler Coplan

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
* Author: Tyler Coplan
* Assignment: 8-3 Coding Collisions
* Date: April 23rd, 2022
* 
* HOW TO PLAY
* Use the A and D keys to move the ship left and right.  Move the mouse
* cursor to change the angle of the shot direction from one of three fixed
* positions (up left, up, or up right).  Left click mouse to shoot.
* 
* RULES
* Red ships are worth one point, missle will destroy ship and not incurr a hit
* Green ships are worth five points, missle will destroy ship and not incurr a hit
* Gold ships are worth 10 points, missle will destroy ship and not incurr a hit
* 
* Asteroids are reflective, will bounce missle in the appropriate direction
* from the collision angle and missle will incurr a hit
* Walls are reflective, will bounce missle in the appropriate direction from
* the collision angle and missle will incurr a hit
* 
* Missle will bounce twice and be destroyed on third hit, red missle has two bounces
* remaining, green missle one bounce and white missle will be destroyed on impact
* with asteroid or wall.
* 
* Collisions with missles, asteroids or ships will destroy player ship, causing 
* a destroyed spaceship animation and disabled controls (press ESC to end game).
*/
#include <GLFW\glfw3.h>
#include "linmath.h"
#include <stdlib.h>
#include <stdio.h>
#include <conio.h>
#include <iostream>
#include <vector>
#include <windows.h>
#include <time.h>

using namespace std;

const float DEG2RAD = 3.14159 / 180;
const int DIRECTION = 3;

// will get incremented in the draw stars function, just another hacky solution
// ultimately gets used to move stars and seed random function because srand(time(NULL)) is not making random results
static float counterForThings = 0;

// track player score
static int playerScore = 0;

bool endGame = false;
void processInput(GLFWwindow* window);

enum BRICKTYPE { REFLECTIVE, DESTRUCTABLE };
enum ONOFF { ON, OFF };

class Brick
{
public:
	float red, green, blue;
	float x, y, width;
	int score;
	float speed = 0.01;
	BRICKTYPE brick_type;
	ONOFF onoff;

	Brick(BRICKTYPE bt, float xx, float yy, float ww, float rr, float gg, float bb, int points)
	{
		brick_type = bt; x = xx; y = yy, width = ww; red = rr, green = gg, blue = bb;
		score = points;
		onoff = OFF;
	};

	void moveBrick()
	{
		if (onoff == ON)
		{
			if (y < -1.0)
				onoff = OFF;
			else
				y -= speed;
		}
	}
	void drawAsteroid()
	{
		if (onoff == ON)
		{
			glColor3f(0.8, 0.8, 0.8);
			glBegin(GL_POLYGON);
			for (int i = 0; i <= 360; i = i + 36)
			{	
				float degInRad = i * DEG2RAD;
				glVertex2f((cos(degInRad) * width) + x, (sin(degInRad) * width) + y);
			}
			glEnd();
		}
	}
	void drawEnemy()
	{
		if (onoff == ON)
		{
			glColor3d(red, green, blue);
			glBegin(GL_POLYGON);
			glVertex2d(x - width / 4, y + width/2);
			glVertex2d(x, y - width/2);
			glVertex2d(x + width / 4, y + width / 2);
			glEnd();
			glBegin(GL_POLYGON);
			glVertex2d(x - width / 2, y + width / 2);
			glVertex2d(x - width / 3, y);
			glVertex2d(x - width / 4, y + width / 2);
			glEnd();
			glBegin(GL_POLYGON);
			glVertex2d(x + width / 2, y + width / 2);
			glVertex2d(x + width / 3, y);
			glVertex2d(x + width / 4, y + width / 2);
			glEnd();
		}
	}
};


class Missle
{
public:
	float red, green, blue;
	float radius;
	float x;
	float y;
	float speed = 0.03;
	int HP = 3;
	int direction; // 1=down 2=right 3=up 4=left  5 = down right   6 = down left  7 = up right  8= up left

	Missle(double xx, double yy, double rr, int dir, float rad, float r, float g, float b)
	{
		x = xx;
		y = yy;
		radius = rr;
		red = r;
		green = g;
		blue = b;
		radius = rad;
		direction = dir;
	}

	void setColor()
	{
		if (HP == 3)
		{
			red = 1;
			green = 0;
			blue = 0;
		}

		else if (HP == 2)
		{
			red = 0;
			green = 1;
			blue = 0;
		}

		else if (HP == 1)
		{
			red = green = blue = 1;
		}

		// sloppy solution to destroy a missle
		else
		{
			x = 10;
			y = 10;
			speed = 0;
			HP = 0;
		}
	}

	void CheckCollision(Brick* brk)
	{
		if (brk->brick_type == REFLECTIVE)
		{
			// collision has occured
			if ((x > brk->x - brk->width && x <= brk->x + brk->width) && (y > brk->y - brk->width && y <= brk->y + brk->width))
			{

				// these branches apply the logic to control bounce angle
				if (direction == 1)
				{
					direction = 3;
					y += 0.02;
				}

				else if (direction == 2)
				{
					direction = 4;
					x -= 0.02;
				}

				else if (direction == 3)
				{
					direction = 1;
					y -= 0.02;
				}

				else if (direction == 4)
				{
					direction = 2;
					x += 0.02;
				}

				else if (direction == 5)
				{
					direction = 7;
					x += 0.012;
					y += 0.012;
				}

				else if (direction == 6)
				{
					// go up left
					direction = 8;
					x -= 0.012;
					y += 0.012;

				}

				else if (direction == 7)
				{
					// did it hit the bottom?
					if (y >= brk->y - brk->width && y <= brk->y - brk->width + 0.05)
					{
						// go down right
						float bottom = brk->y - brk->width;
						direction = 5;
						x += 0.012;
						y -= 0.05;
					}
					else
					{
						// go up left
						float bottom = brk->y - brk->width;
						direction = 8;
						x -= 0.012;
						y += 0.012;
					}
				}

				else if (direction == 8)
				{
					// did it hit the bottom?
					if (y >= brk->y - brk->width && y <= brk->y - brk->width + 0.05)
					{
						// go down left
						direction = 6;
						x -= 0.012;
						y -= 0.05;
					}
					else
					{
						direction = 7;
						x += 0.012;
						y += 0.012;
					}
				}

				// 1=down 2=right 3=up 4=left  5 = down right   6 = down left  7 = up right  8= up left
				// compute missle hit
				HP--;
				setColor();
			}
		}
		else if (brk->brick_type == DESTRUCTABLE)
		{
			if ((x > brk->x - brk->width && x <= brk->x + brk->width) && (y > brk->y - brk->width && y <= brk->y + brk->width))
			{
				brk->onoff = OFF;
				playerScore += brk->score;
				cout << "SCORE: " << playerScore << endl;
			}
		}
	}

	void MoveOneStep()
	{
		if (direction == 1 || direction == 5 || direction == 6)  // up
		{
			// did not hit wall
			if (y > -1 + radius)
			{
				y -= speed;
			}

			// hit wall
			else
			{
				HP--;
				setColor();
				if (direction == 1)
					direction = 3;
				else if (direction == 6)
					direction = 8;
				else if (direction == 5)
					direction = 7;
			}
		}

		if (direction == 2 || direction == 5 || direction == 7)  // right
		{
			// did not hit wall
			if (x < 1 - radius)
			{
				x += speed;
			}
			// hit wall
			else
			{
				HP--;
				setColor();
				if (direction == 5)
					direction = 6;
				else if (direction == 7)
					direction = 8;
				else if (direction == 2)
					direction = 4;

			}
		}

		if (direction == 3 || direction == 7 || direction == 8)  // down
		{
			if (y < 1 - radius) {
				y += speed;
			}
			else
			{
				HP--;
				setColor();
				if (direction == 3)
					direction = 1;
				else if (direction == 8)
					direction = 6;
				else if (direction == 7)
					direction = 5;
			}
		}

		if (direction == 4 || direction == 6 || direction == 8)  // left
		{
			if (x > -1 + radius) {
				x -= speed;
			}
			else
			{
				HP--;
				setColor();
				if (direction == 6)
					direction = 5;
				else if (direction == 8)
					direction = 7;
				else if (direction == 4)
					direction = 2;
			}
		}
	}

	void DrawMissle()
	{
		glColor3f(red, green, blue);
		glBegin(GL_POLYGON);
		for (int i = 0; i <= 360; i = i + 36) {
			float degInRad = i * DEG2RAD;
			glVertex2f((cos(degInRad) * radius) + x, (sin(degInRad) * radius) + y);

			// missle going up/ down
			if (direction == 1 || direction == 3)
				glVertex2f((cos(degInRad) * radius) + x, (sin(degInRad) * radius) + y + 0.1);

			// missle going down-left / up right
			else if (direction == 5 || direction == 8)
				glVertex2f((cos(degInRad) * radius) + x - 0.07, (sin(degInRad) * radius) + y + 0.07);

			// missle going down right / up left
			else if (direction == 6 || direction == 7)
				glVertex2f((cos(degInRad) * radius) + x + 0.07, (sin(degInRad) * radius) + y + 0.07);

			// missle going left/right
			else if (direction == 2 || direction == 4)
				glVertex2f((cos(degInRad) * radius) + x - 0.1, (sin(degInRad) * radius) + y);
		}

		// draw missle depnding on direction
		if (direction == 1 || direction == 3)
		{
			glVertex2f(x + radius, y - radius);
			glVertex2f(x + radius, y - radius + 0.1);
			glVertex2f(x - radius, y - radius + 0.1);
			glVertex2f(x - radius, y - radius);
		}

		else if (direction == 5 || direction == 8)
		{
			glVertex2f(x + radius, y - radius);
			glVertex2f(x + radius - 0.07, y - radius + 0.07);
			glVertex2f(x - radius - 0.07, y - radius + 0.07);
			glVertex2f(x - radius, y - radius);
		}

		else if (direction == 6 || direction == 7)
		{
			glVertex2f(x + radius, y - radius);
			glVertex2f(x + radius + 0.07, y - radius + 0.07);
			glVertex2f(x - radius + 0.07, y - radius + 0.07);
			glVertex2f(x - radius, y - radius);
		}

		else if (direction == 2 || direction == 4)
		{
			glVertex2f(x + radius, y + radius);
			glVertex2f(x + radius, y - radius);
			glVertex2f(x - 0.1, y - radius);
			glVertex2f(x - 0.1, y + radius);
		}
		glEnd();
	}
};

class Spaceship
{
public:
	float xOffset;
	int direction;

	Spaceship(float x, int dir)
	{
		// offset value changed by user controls and moves the spaceship
		xOffset = x;
		direction = dir;
	}

	void DrawSpaceship()
	{
		// draw cockpit
		glColor3f(0.6, 0.6, 0.6);
		glBegin(GL_POLYGON);
		for (int i = 0; i <= 360; i = i + 36) {
			float degInRad = i * DEG2RAD;
			glVertex2f((cos(degInRad) * 0.1) + xOffset, (sin(degInRad) * 0.1) + -0.8);
		}
		glEnd();

		// draw left wing
		glBegin(GL_POLYGON);
		for (int i = 0; i < 360; i++) {
			float degInRad = i * DEG2RAD;
			glVertex2f((cos(degInRad) * 0.1) - 0.3 + xOffset, (sin(degInRad) * 0.1) + -0.8);
		}
		glEnd();

		// draw right wing
		glBegin(GL_POLYGON);
		for (int i = 0; i < 360; i++) {
			float degInRad = i * DEG2RAD;
			glVertex2f((cos(degInRad) * 0.1) + 0.3 + xOffset, (sin(degInRad) * 0.1) + -0.8);
		}
		glEnd();
		glBegin(GL_POLYGON);

		// draw black circles to void out some color on left wing
		glColor3f(0.0, 0.0, 0.0);
		for (int i = 0; i < 360; i++) {
			float degInRad = i * DEG2RAD;
			glVertex2f((cos(degInRad) * 0.1) + 0.25 + xOffset, (sin(degInRad) * 0.1) + -0.8);
		}
		glEnd();

		// draw black circles to void out some color on right wing
		glBegin(GL_POLYGON);
		for (int i = 0; i < 360; i++) {
			float degInRad = i * DEG2RAD;
			glVertex2f((cos(degInRad) * 0.1) - 0.25 + xOffset, (sin(degInRad) * 0.1) + -0.8);
		}
		glEnd();
		glBegin(GL_POLYGON);

		// draw arms to wings
		glColor3f(0.6, 0.6, 0.6);
		glVertex2d(0.35 + xOffset, -0.82);
		glVertex2d(0.35 + xOffset, -0.78);
		glVertex2d(-0.35 + xOffset, -0.78);
		glVertex2d(-0.35 + xOffset, -0.82);
		glEnd();

		// draw laser cannon
		glBegin(GL_POLYGON);
		if (direction == 7)
		{
			glVertex2d(xOffset + 0.01, -0.7);
			glVertex2d(xOffset + 0.1, -0.63);
			glVertex2d(xOffset + 0.08, -0.61);
			glVertex2d(xOffset - 0.01, -0.7);
		}

		else if (direction == 8)
		{
			glVertex2d(xOffset + 0.01, -0.7);
			glVertex2d(xOffset - 0.1, -0.63);
			glVertex2d(xOffset - 0.08, -0.61);
			glVertex2d(xOffset - 0.01, -0.7);
		}

		else
		{
			glVertex2d(xOffset + 0.01, -0.7);
			glVertex2d(xOffset + 0.01, -0.6);
			glVertex2d(xOffset - 0.01, -0.6);
			glVertex2d(xOffset - 0.01, -0.7);
		}

		glEnd();
	}

	void DrawSpaceshipDead()
	{
		// draw cockpit
		glColor3f(0.6, 0.6, 0.6);
		glBegin(GL_POLYGON);
		for (int i = 0; i <= 280; i = i + 36) {
			float degInRad = i * DEG2RAD;
			glVertex2f((cos(degInRad) * 0.1) + xOffset, (sin(degInRad) * 0.1) + -0.8);
		}
		glEnd();

		// draw left wing
		glBegin(GL_POLYGON);
		for (int i = 360; i > 120; i--) {
			float degInRad = i * DEG2RAD;
			glVertex2f((cos(degInRad) * 0.1) - 0.3 + xOffset, (sin(degInRad) * 0.1) + -0.8);
		}
		glEnd();

		// draw right wing
		glBegin(GL_POLYGON);
		for (int i = 0; i < 360; i++) {
			float degInRad = i * DEG2RAD;
			glVertex2f((cos(degInRad) * 0.1) + 0.3 + xOffset, (sin(degInRad) * 0.1) + -0.8);
		}
		glEnd();
		glBegin(GL_POLYGON);

		// draw black circles to void out some color on left wing
		glColor3f(0.0, 0.0, 0.0);
		for (int i = 0; i < 360; i++) {
			float degInRad = i * DEG2RAD;
			glVertex2f((cos(degInRad) * 0.1) + 0.25 + xOffset, (sin(degInRad) * 0.1) + -0.8);
		}
		glEnd();

		// draw black circles to void out some color on right wing
		glBegin(GL_POLYGON);
		for (int i = 0; i < 360; i++) {
			float degInRad = i * DEG2RAD;
			glVertex2f((cos(degInRad) * 0.1) - 0.25 + xOffset, (sin(degInRad) * 0.1) + -0.8);
		}
		glEnd();
		glBegin(GL_POLYGON);

		// draw arms to wings
		glColor3f(0.6, 0.6, 0.6);
		glVertex2d(0.35 + xOffset, -0.82);
		glVertex2d(0.35 + xOffset, -0.78);
		glVertex2d(-0.35 + xOffset, -0.82);
		glVertex2d(-0.35 + xOffset, -0.82);
		glEnd();

		// draw laser cannon
		glBegin(GL_POLYGON);
		if (direction == 7)
		{
			glVertex2d(xOffset + 0.01, -0.7);
			glVertex2d(xOffset + 0.1, -0.63);
			glVertex2d(xOffset + 0.08, -0.61);
			glVertex2d(xOffset - 0.01, -0.7);
		}

		else if (direction == 8)
		{
			glVertex2d(xOffset + 0.01, -0.7);
			glVertex2d(xOffset - 0.1, -0.63);
			glVertex2d(xOffset - 0.08, -0.61);
			glVertex2d(xOffset - 0.01, -0.7);
		}

		else
		{
			glVertex2d(xOffset + 0.01, -0.7);
			glVertex2d(xOffset + 0.01, -0.6);
			glVertex2d(xOffset - 0.01, -0.6);
			glVertex2d(xOffset - 0.01, -0.7);
		}

		glEnd();
	}

	void detectCollision(vector<Missle> missles, vector<Brick> bricks)
	{
		int missleCount = missles.size();
		int bricksCount = bricks.size();

		for (int i = 0; i < bricksCount; i++)
		{
			if (bricks.at(i).onoff == ON)
			{
				if (bricks.at(i).x + bricks.at(i).width > xOffset - 0.35 &&
					bricks.at(i).x - bricks.at(i).width < xOffset + 0.35 &&
					bricks.at(i).y - bricks.at(i).width <= -0.7 &&
					bricks.at(i).y + bricks.at(i).width >= -0.8)
				{
					endGame = true;
				}
			}
		}

		for (int i = 0; i < missleCount; i++)
		{
			if (missles.at(i).x + missles.at(i).radius > xOffset - 0.35 &&
				missles.at(i).x - missles.at(i).radius < xOffset + 0.35 &&
				missles.at(i).y - missles.at(i).radius <= -0.7 &&
				missles.at(i).y + missles.at(i).radius >= -0.8)
			{
				endGame = true;
			}
		}
	}
};

void drawStars()
{
	float x, y;
	srand(1);
	for (int i = 0; i < 1000; i++)
	{
		glBegin(GL_POLYGON);
		glColor3f(1.0, 1.0, 1.0);
		x = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / 1.9)) - 0.95;
		y = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / 50)) - 0.8;
		glVertex2f(x, y - counterForThings);
		glVertex2f(x, y - counterForThings - 0.01);
		glVertex2f(x + 0.01, y - counterForThings - 0.01);
		glVertex2f(x + 0.01, y - counterForThings);
		glEnd();
	}
	counterForThings += 0.0025;
	srand(time(NULL));
}

void drawScoreboard()
{
	// set color
	glColor3f(1.0f, 1.0f, 1.0f);

	// draw S
	glBegin(GL_POLYGON);
	glVertex2f(0.1, 0.95);
	glVertex2f(0.15, 0.95);
	glVertex2f(0.15, 0.94);
	glVertex2f(0.1, 0.94);
	glEnd();
	glBegin(GL_POLYGON);
	glVertex2f(0.1, 0.94);
	glVertex2f(0.1, 0.90);
	glVertex2f(0.11, 0.90);
	glVertex2f(0.11, 0.94);
	glEnd();
	glBegin(GL_POLYGON);
	glVertex2f(0.1, 0.90);
	glVertex2f(0.15, 0.90);
	glVertex2f(0.15, 0.89);
	glVertex2f(0.1, 0.89);
	glEnd();
	glBegin(GL_POLYGON);
	glVertex2f(0.14, 0.90);
	glVertex2f(0.15, 0.90);
	glVertex2f(0.15, 0.85);
	glVertex2f(0.14, 0.85);
	glEnd();
	glBegin(GL_POLYGON);
	glVertex2f(0.1, 0.85);
	glVertex2f(0.15, 0.85);
	glVertex2f(0.15, 0.84);
	glVertex2f(0.1, 0.84);
	glEnd();
	// draw C
	glBegin(GL_POLYGON);
	glVertex2f(0.25, 0.95);
	glVertex2f(0.20, 0.95);
	glVertex2f(0.2, 0.94);
	glVertex2f(0.25, 0.94);
	glEnd();
	glBegin(GL_POLYGON);
	glVertex2f(0.2, 0.95);
	glVertex2f(0.21, 0.95);
	glVertex2f(0.21, 0.84);
	glVertex2f(0.20, 0.84);
	glEnd();
	glBegin(GL_POLYGON);
	glVertex2f(0.2, 0.85);
	glVertex2f(0.2, 0.84);
	glVertex2f(0.25, 0.84);
	glVertex2f(0.25, 0.85);
	glEnd();

	// draw 0
	glBegin(GL_POLYGON);
	glVertex2f(0.35, 0.95);
	glVertex2f(0.30, 0.95);
	glVertex2f(0.3, 0.94);
	glVertex2f(0.35, 0.94);
	glEnd();
	glBegin(GL_POLYGON);
	glVertex2f(0.3, 0.95);
	glVertex2f(0.31, 0.95);
	glVertex2f(0.31, 0.84);
	glVertex2f(0.30, 0.84);
	glEnd();
	glBegin(GL_POLYGON);
	glVertex2f(0.3, 0.85);
	glVertex2f(0.3, 0.84);
	glVertex2f(0.35, 0.84);
	glVertex2f(0.35, 0.85);
	glEnd();
	glBegin(GL_POLYGON);
	glVertex2f(0.34, 0.84);
	glVertex2f(0.35, 0.84);
	glVertex2f(0.35, 0.95);
	glVertex2f(0.34, 0.95);
	glEnd();

	// draw R
	glBegin(GL_POLYGON);
	glVertex2f(0.45, 0.94);
	glVertex2f(0.45, 0.95);
	glVertex2f(0.40, 0.95);
	glVertex2f(0.40, 0.94);
	glEnd();

	glBegin(GL_POLYGON);
	glVertex2f(0.41, 0.95);
	glVertex2f(0.41, 0.84);
	glVertex2f(0.40, 0.84);
	glVertex2f(0.40, 0.95);
	glEnd();

	glBegin(GL_POLYGON);
	glVertex2f(0.44, 0.95);
	glVertex2f(0.44, 0.90);
	glVertex2f(0.45, 0.90);
	glVertex2f(0.45, 0.95);
	glEnd();

	glBegin(GL_POLYGON);
	glVertex2f(0.45, 0.90);
	glVertex2f(0.45, 0.89);
	glVertex2f(0.40, 0.89);
	glVertex2f(0.40, 0.90);
	glEnd();

	glBegin(GL_POLYGON);
	glVertex2f(0.40, 0.89);
	glVertex2f(0.41, 0.90);
	glVertex2f(0.45, 0.86);
	glVertex2f(0.45, 0.84);
	glEnd();

	// Draw E
	glBegin(GL_POLYGON);
	glVertex2f(0.51, 0.95);
	glVertex2f(0.51, 0.84);
	glVertex2f(0.50, 0.84);
	glVertex2f(0.50, 0.95);
	glEnd();

	glBegin(GL_POLYGON);
	glVertex2f(0.50, 0.95);
	glVertex2f(0.50, 0.94);
	glVertex2f(0.55, 0.94);
	glVertex2f(0.55, 0.95);
	glEnd();

	glBegin(GL_POLYGON);
	glVertex2f(0.50, 0.90);
	glVertex2f(0.50, 0.89);
	glVertex2f(0.55, 0.89);
	glVertex2f(0.55, 0.90);
	glEnd();

	glBegin(GL_POLYGON);
	glVertex2f(0.50, 0.85);
	glVertex2f(0.50, 0.84);
	glVertex2f(0.55, 0.84);
	glVertex2f(0.55, 0.85);
	glEnd();
}

void drawNumbers(int Position, int number)
{
	float offset = 0;
	if (Position == 1)
		offset = 0.9;
	else if (Position == 2)
		offset = 0.8;
	else if (Position == 3)
		offset = 0.7;
	else if (Position == 4)
		offset = 0.6;
	else
		offset = 42;

	if (number == 1)
	{
		glBegin(GL_POLYGON);
		glVertex2f(0.05 + offset, 0.85);
		glVertex2f(0.04 + offset, 0.85);
		glVertex2f(0.04 + offset, 0.95);
		glVertex2f(0.05 + offset, 0.95);
		glEnd();
	}
	else if (number == 2)
	{
		glBegin(GL_POLYGON);
		glVertex2f(offset, 0.95);
		glVertex2f(0.05 + offset, 0.95);
		glVertex2f(0.05 + offset, 0.94);
		glVertex2f(offset, 0.94);
		glEnd();
		glBegin(GL_POLYGON);
		glVertex2f(0.04 + offset, 0.94);
		glVertex2f(0.04 + offset, 0.90);
		glVertex2f(0.05 + offset, 0.90);
		glVertex2f(0.05 + offset, 0.94);
		glEnd();
		glBegin(GL_POLYGON);
		glVertex2f(offset, 0.90);
		glVertex2f(0.05 + offset, 0.90);
		glVertex2f(0.05 + offset, 0.89);
		glVertex2f(offset, 0.89);
		glEnd();
		glBegin(GL_POLYGON);
		glVertex2f(offset, 0.90);
		glVertex2f(0.01 + offset, 0.90);
		glVertex2f(0.01 + offset, 0.85);
		glVertex2f(+offset, 0.85);
		glEnd();
		glBegin(GL_POLYGON);
		glVertex2f(offset, 0.85);
		glVertex2f(0.05 + offset, 0.85);
		glVertex2f(0.05 + offset, 0.84);
		glVertex2f(offset, 0.84);
		glEnd();
	}

	else if (number == 3)
	{
		// Draw 3
		glBegin(GL_POLYGON);
		glVertex2f(offset + 0.05, 0.95);
		glVertex2f(offset + 0.05, 0.84);
		glVertex2f(offset + 0.04, 0.84);
		glVertex2f(offset + 0.04, 0.95);
		glEnd();

		glBegin(GL_POLYGON);
		glVertex2f(offset, 0.95);
		glVertex2f(offset, 0.94);
		glVertex2f(offset + 0.05, 0.94);
		glVertex2f(offset + 0.05, 0.95);
		glEnd();

		glBegin(GL_POLYGON);
		glVertex2f(offset, 0.90);
		glVertex2f(offset, 0.89);
		glVertex2f(offset + 0.05, 0.89);
		glVertex2f(offset + 0.05, 0.90);
		glEnd();

		glBegin(GL_POLYGON);
		glVertex2f(offset, 0.85);
		glVertex2f(offset, 0.84);
		glVertex2f(offset + 0.05, 0.84);
		glVertex2f(offset + 0.05, 0.85);
		glEnd();
	}

	else if (number == 4)
	{
		glBegin(GL_POLYGON);
		glVertex2f(offset, 0.95);
		glVertex2f(0.01 + offset, 0.95);
		glVertex2f(0.01 + offset, 0.9);
		glVertex2f(offset, 0.9);
		glEnd();

		glBegin(GL_POLYGON);
		glVertex2f(offset, 0.90);
		glVertex2f(offset, 0.89);
		glVertex2f(0.05 + offset, 0.89);
		glVertex2f(offset + 0.05, 0.90);
		glEnd();

		glBegin(GL_POLYGON);
		glVertex2f(offset + 0.05, 0.95);
		glVertex2f(offset + 0.04, 0.95);
		glVertex2f(0.04 + offset, 0.84);
		glVertex2f(offset + 0.05, 0.84);
		glEnd();
	}

	else if (number == 5)
	{
		glBegin(GL_POLYGON);
		glVertex2f(offset, 0.95);
		glVertex2f(offset + 0.05, 0.95);
		glVertex2f(offset + 0.05, 0.94);
		glVertex2f(offset, 0.94);
		glEnd();
		glBegin(GL_POLYGON);
		glVertex2f(offset, 0.94);
		glVertex2f(offset, 0.90);
		glVertex2f(offset + 0.01, 0.90);
		glVertex2f(offset + 0.01, 0.94);
		glEnd();
		glBegin(GL_POLYGON);
		glVertex2f(offset, 0.90);
		glVertex2f(offset + 0.05, 0.90);
		glVertex2f(offset + 0.05, 0.89);
		glVertex2f(offset, 0.89);
		glEnd();
		glBegin(GL_POLYGON);
		glVertex2f(offset + 0.04, 0.90);
		glVertex2f(offset + 0.05, 0.90);
		glVertex2f(offset + 0.05, 0.85);
		glVertex2f(offset + 0.04, 0.85);
		glEnd();
		glBegin(GL_POLYGON);
		glVertex2f(offset, 0.85);
		glVertex2f(offset + 0.05, 0.85);
		glVertex2f(offset + 0.05, 0.84);
		glVertex2f(offset, 0.84);
		glEnd();
	}

	else if (number == 6)
	{
		glBegin(GL_POLYGON);
		glVertex2f(offset, 0.95);
		glVertex2f(offset + 0.05, 0.95);
		glVertex2f(offset + 0.05, 0.94);
		glVertex2f(offset, 0.94);
		glEnd();
		glBegin(GL_POLYGON);
		glVertex2f(offset, 0.94);
		glVertex2f(offset, 0.84);
		glVertex2f(offset + 0.01, 0.84);
		glVertex2f(offset + 0.01, 0.94);
		glEnd();
		glBegin(GL_POLYGON);
		glVertex2f(offset, 0.90);
		glVertex2f(offset + 0.05, 0.90);
		glVertex2f(offset + 0.05, 0.89);
		glVertex2f(offset, 0.89);
		glEnd();
		glBegin(GL_POLYGON);
		glVertex2f(offset + 0.04, 0.90);
		glVertex2f(offset + 0.05, 0.90);
		glVertex2f(offset + 0.05, 0.85);
		glVertex2f(offset + 0.04, 0.85);
		glEnd();
		glBegin(GL_POLYGON);
		glVertex2f(offset, 0.85);
		glVertex2f(offset + 0.05, 0.85);
		glVertex2f(offset + 0.05, 0.84);
		glVertex2f(offset, 0.84);
		glEnd();
	}

	else if (number == 7)
	{
		glBegin(GL_POLYGON);
		glVertex2f(offset, 0.95);
		glVertex2f(offset, 0.94);
		glVertex2f(offset + 0.05, 0.94);
		glVertex2f(offset + 0.05, 0.95);
		glEnd();
		glBegin(GL_POLYGON);
		glVertex2f(offset + 0.05, 0.95);
		glVertex2f(offset + 0.04, 0.95);
		glVertex2f(offset + 0.04, 0.84);
		glVertex2f(offset + 0.05, 0.84);
		glEnd();
	}

	else if (number == 8)
	{
		glBegin(GL_POLYGON);
		glVertex2f(offset, 0.95);
		glVertex2f(offset + 0.05, 0.95);
		glVertex2f(offset + 0.05, 0.94);
		glVertex2f(offset, 0.94);
		glEnd();
		glBegin(GL_POLYGON);
		glVertex2f(offset, 0.94);
		glVertex2f(offset, 0.84);
		glVertex2f(offset + 0.01, 0.84);
		glVertex2f(offset + 0.01, 0.94);
		glEnd();
		glBegin(GL_POLYGON);
		glVertex2f(offset, 0.90);
		glVertex2f(offset + 0.05, 0.90);
		glVertex2f(offset + 0.05, 0.89);
		glVertex2f(offset, 0.89);
		glEnd();
		glBegin(GL_POLYGON);
		glVertex2f(offset + 0.04, 0.95);
		glVertex2f(offset + 0.05, 0.95);
		glVertex2f(offset + 0.05, 0.84);
		glVertex2f(offset + 0.04, 0.84);
		glEnd();
		glBegin(GL_POLYGON);
		glVertex2f(offset, 0.85);
		glVertex2f(offset + 0.05, 0.85);
		glVertex2f(offset + 0.05, 0.84);
		glVertex2f(offset, 0.84);
		glEnd();
	}

	else if (number == 9)
	{
		glBegin(GL_POLYGON);
		glVertex2f(offset, 0.95);
		glVertex2f(offset + 0.05, 0.95);
		glVertex2f(offset + 0.05, 0.94);
		glVertex2f(offset, 0.94);
		glEnd();
		glBegin(GL_POLYGON);
		glVertex2f(offset, 0.94);
		glVertex2f(offset, 0.90);
		glVertex2f(offset + 0.01, 0.90);
		glVertex2f(offset + 0.01, 0.94);
		glEnd();
		glBegin(GL_POLYGON);
		glVertex2f(offset, 0.90);
		glVertex2f(offset + 0.05, 0.90);
		glVertex2f(offset + 0.05, 0.89);
		glVertex2f(offset, 0.89);
		glEnd();
		glBegin(GL_POLYGON);
		glVertex2f(offset + 0.04, 0.95);
		glVertex2f(offset + 0.05, 0.95);
		glVertex2f(offset + 0.05, 0.85);
		glVertex2f(offset + 0.04, 0.85);
		glEnd();
		glBegin(GL_POLYGON);
		glVertex2f(offset, 0.85);
		glVertex2f(offset + 0.05, 0.85);
		glVertex2f(offset + 0.05, 0.84);
		glVertex2f(offset, 0.84);
		glEnd();
	}

	else if (number == 0)
	{
		glBegin(GL_POLYGON);
		glVertex2f(offset, 0.95);
		glVertex2f(offset + 0.05, 0.95);
		glVertex2f(offset + 0.05, 0.94);
		glVertex2f(offset, 0.94);
		glEnd();
		glBegin(GL_POLYGON);
		glVertex2f(offset, 0.94);
		glVertex2f(offset, 0.84);
		glVertex2f(offset + 0.01, 0.84);
		glVertex2f(offset + 0.01, 0.94);
		glEnd();
		glBegin(GL_POLYGON);
		glVertex2f(offset + 0.04, 0.95);
		glVertex2f(offset + 0.05, 0.95);
		glVertex2f(offset + 0.05, 0.84);
		glVertex2f(offset + 0.04, 0.84);
		glEnd();
		glBegin(GL_POLYGON);
		glVertex2f(offset, 0.85);
		glVertex2f(offset + 0.05, 0.85);
		glVertex2f(offset + 0.05, 0.84);
		glVertex2f(offset, 0.84);
		glEnd();
	}
}

// create global array for missles and bricks and a global pointer to the spacehsip
vector<Missle> world;
Spaceship* shipPtr = nullptr;
vector<Brick> bricks;

int main(void) {
	srand(time(NULL));

	// initialize window
	if (!glfwInit()) {
		exit(EXIT_FAILURE);
	}
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
	GLFWwindow* window = glfwCreateWindow(800, 800, "Space Shooter", NULL, NULL);
	if (!window) {
		glfwTerminate();
		exit(EXIT_FAILURE);
	}
	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);

	// turn cursor into crosshair
	GLFWcursor* cursor = glfwCreateStandardCursor(GLFW_CROSSHAIR_CURSOR);
	glfwSetCursor(window, cursor);

	// make 50 asteroids
	for (int i = 0; i < 50; i++)
	{
		float xRand = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / 1.8)) - 0.9;
		Brick brick(REFLECTIVE, xRand, -1.0, 0.1, 0.8, 0.8, 0.8, 0);
		bricks.push_back(brick);
	}

	// make 40 1 point enemy ships
	for (int i = 0; i < 40; i++)
	{
		float xRand = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / 1.8)) - 0.9;
		Brick brick(DESTRUCTABLE, xRand, -1.0, 0.1, 1.0, 0.0, 0.0, 1);
		bricks.push_back(brick);
	}

	// make 9 5 point enemy ships
	for (int i = 0; i < 9; i++)
	{
		float xRand = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / 1.8)) - 0.9;
		Brick brick(DESTRUCTABLE, xRand, -1.0, 0.1, 0.0, 1.0, 0.0, 5);
		bricks.push_back(brick);
	}

	// make 1 10 point enemy ship
	float xRand = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / 1.8)) - 0.9;
	Brick goldBrick(DESTRUCTABLE, xRand, -1.0, 0.1, 0.8, 0.7, 0.2, 2);
	bricks.push_back(goldBrick);

	// make spacehip and point global pointer to the spaceship
	Spaceship spaceship(0, DIRECTION);
	shipPtr = &spaceship;

	float lastFrame = glfwGetTime();
	float deltaTime = 0;
	while (!glfwWindowShouldClose(window)) {

		deltaTime = deltaTime + glfwGetTime() - lastFrame;
		lastFrame = glfwGetTime();

		//Setup View
		float ratio;
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);
		ratio = width / (float)height;
		glViewport(0, 0, width, height);
		glClear(GL_COLOR_BUFFER_BIT);

		processInput(window);

		drawStars();
		//Movement
		for (int i = 0; i < world.size(); i++)
		{
			for (int j = 0; j < 100; j++)
				if (bricks.at(j).onoff == ON)
					world[i].CheckCollision(&bricks.at(j));
			world[i].MoveOneStep();
			world[i].DrawMissle();

		}

		// generate a new brick at the top of the screen every second
		if (deltaTime > 1)
		{	
			srand(counterForThings * 12547);  // just some randomness, well aware that a program that ran for a long time could overflow the arguments memory
			int index = rand() % bricks.size();;  //
			bricks.at(index).onoff = ON;
			bricks.at(index).y = 1.0;
			bricks.at(index).x = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / 1.8)) - 0.9;
			deltaTime = 0;
		}

		// draw asteroids/enemy ships
		for (int i = 0; i < bricks.size(); i++)
		{
			if (i <= 50)
				bricks.at(i).drawAsteroid();
			else
				bricks.at(i).drawEnemy();
			if (bricks.at(i).onoff == ON)
				bricks.at(i).moveBrick();
		}

		endGame ? spaceship.DrawSpaceshipDead() : spaceship.DrawSpaceship();
		spaceship.detectCollision(world, bricks);
		drawScoreboard();

		if (playerScore >= 1000)
		{
			int printNum = (playerScore % 10000) / 1000;
			drawNumbers(4, printNum);
		}

		if (playerScore >= 100)
		{
			int printNum = (playerScore % 1000) / 100;
			drawNumbers(3, printNum);
		}

		if (playerScore >= 10)
		{
			int printNum = (playerScore % 100) / 10;
			drawNumbers(2, printNum);
		}

		drawNumbers(1, playerScore % 10);

		glfwSwapBuffers(window);
		glfwPollEvents();

	}

	glfwDestroyCursor(cursor);
	glfwDestroyWindow(window);
	glfwTerminate;
	exit(EXIT_SUCCESS);
}

// SOURCE https://www.glfw.org/docs/3.3/input_guide.html
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS && !endGame)
	{
		if (world.size() < 5)
		{
			double aimOffset = 0;
			if (shipPtr->direction == 8)
				aimOffset = -0.1;
			if (shipPtr->direction == 7)
				aimOffset = 0.1;
			Missle B(shipPtr->xOffset + aimOffset, -0.6, 02, shipPtr->direction, 0.01, 1, 0, 0);
			world.insert(world.begin(), B);
		}

		else
		{
			for (int i = 0; i < 5; i++)
			{
				if (world.at(i).HP == 0)
				{
					double aimOffset = 0;
					if (shipPtr->direction == 8)
						aimOffset = -0.1;
					if (shipPtr->direction == 7)
						aimOffset = 0.1;
					world.at(i).x = shipPtr->xOffset + aimOffset;
					world.at(i).y = -0.6;
					world.at(i).direction = shipPtr->direction;
					world.at(i).radius = 0.01;
					world.at(i).blue = 0;
					world.at(i).green = 0;
					world.at(i).HP = 3;
					world.at(i).speed = 0.03;
					break;
				}


				//	Missle B(shipPtr->xOffset + aimOffset, -0.6, 02, shipPtr->direction, 0.01, 1, 0, 0);
				//	world.insert(world.begin(), B);
				//	world.pop_back();
				//}
			}
		}
	}

}

// SOURCE https://www.glfw.org/docs/3.3/input_guide.html
static void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (endGame)
		return;
	if (xpos < 800 / 3)
		shipPtr->direction = 8;
	else if (xpos > 2 * 800 / 3)
		shipPtr->direction = 7;
	else
		shipPtr->direction = 3;

	if (xpos < 200)
		glfwSetCursorPos(window, 200, ypos);

	if (xpos > 600)
		glfwSetCursorPos(window, 600, ypos);
}

void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	// move ship left if left arrow is down                                              // dont move ship if its too far left
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS && !endGame)
		shipPtr->xOffset = (shipPtr->xOffset > -0.6) ? shipPtr->xOffset - 0.02 : shipPtr->xOffset;

	// move ship right if right arrow is down                                       // dont move ship if too far right
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS && !endGame)
		shipPtr->xOffset = (shipPtr->xOffset < 0.6) ? shipPtr->xOffset + 0.02 : shipPtr->xOffset;

	glfwSetCursorPosCallback(window, cursor_position_callback);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
}
