#include "Slingshot.h"
//include above is essential
#include "measure.h"
#include "math.h"

int singleMoveLimit = 20;
int MStep = 6;

int x = 0;
int y = 0;

int totalX = 0;
int totalY = 0;

void p2c(double l,double a,int* x,int* y);
void moveTo(int xNew, int yNew);

void moveBack()
{
        //Mouse.move(0-totalX, totalY);
        moveTo(0, 0);
        totalX = 0;
        totalY = 0;
}

int limitSingleMove(int move)
{
	if(move < (0 - singleMoveLimit) )
		move = 0 - singleMoveLimit;
	if(move > singleMoveLimit)
		move = singleMoveLimit;
	return move;
}

void moveWithSlingshot(double lengthoffset)
{
        double angle;
	double length;
	int xNew;
	int yNew;

	//get position
	length = getLength() - lengthoffset;
        angle = getAngleRad();
	//calculate coordinate
	p2c(length, angle, &xNew, &yNew);
	
	xNew = x + limitSingleMove(xNew - x);
	yNew = y + limitSingleMove(yNew - y);
	
	//move to (xNew,yNew)
	moveTo(xNew, yNew);
	//update current coordinate
	x = xNew;
	y = yNew;
}

void p2c(double l,double a,int* x,int* y)
{
	*x = l*cos(a);
	*y = l*sin(a);
}

void moveTo(int xNew, int yNew)
{
	int moveX = 0;
	int moveY = 0;
	
	boolean lockX = false;
	boolean lockY = false;
	
	moveX = x-xNew;
	moveY = yNew - y;
	
	//record every step we make
	totalX += moveX;
	totalY += moveX;
	
	while(1)
	{
		if(!lockX)
		{
			if (moveX > MStep)
			{
				Mouse.move(MStep, 0);
				moveX -= MStep;
			}
			else if (moveX < (0-MStep))
			{
				Mouse.move(0-MStep, 0);
				moveX += MStep;
			}
			else
			{
				lockX = true;
			}
		}
		
		if(!lockY)
		{
			if (moveY > MStep)
			{
				Mouse.move(0, MStep);
				moveY -= MStep;
			}
			else if (moveY < (0-MStep))
			{
				Mouse.move(0, 0-MStep);
				moveY += MStep;
			}
			else
			{
				lockY = true;
			}
		}
		if(lockX && lockY)
		{
			break;
		}
	}
	
	Mouse.move(moveX, moveY);
	
}

void resetOrigin()
{
  x = 0;
  y = 0;
}
