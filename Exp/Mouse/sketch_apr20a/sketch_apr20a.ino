int i = 10;
int x = 0;
int y = 0;
int totalX = 0;
int totalY = 0;
int MStep = 3;

boolean sw = true;

void setup()
{
  Mouse.begin();
  delay(3000);
}

void loop()
{
  if(i>0)
  {
    moveTo(100,100);
    delay(500);
    moveTo(-100,-100);
    delay(500);
  i--;
  }
  delay(1000);
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
