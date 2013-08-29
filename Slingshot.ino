#include "Slingshot.h"
#include "HX711.h"
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
//include above is essential
#include "movement.h"
#include "measure.h"

//status control : wait/aim/fire
char status = 'w';
int bPin = 3;
bool firstmove = true;
int move_cnt = 0;
double length_offset_fixed = 0;

void setup()
{
	Serial.begin(9600);
	pinMode(bPin, INPUT);
	initHX711();
	initnRF24L01();
	Mouse.begin();
	attachInterrupt(0,special_act, FALLING);
}
void loop()
{
        Serial.println(status);
        
        getForce();
        
        getAngle();
        
	if(status == 'w')
	{
		if(Mouse.isPressed())
		{
			Mouse.release();
		}

		if(tight())
		{
			resetOrigin();
			Mouse.press();
			firstmove = true;
			length_offset_fixed = getLength_offset();
			status = 'a';
		}

	}
	if (status == 'a')
	{
		if(suddenFall())
		{
			Mouse.release();
			delay(20);
			firstmove = true;
			status = 'f';
		}
		else
		{
			 if(firstmove)
			{
			  //Serial.println("1st");
			  moveWithSlingshot(length_offset_fixed);//ACTION  4
			  move_cnt ++;
			  if(move_cnt > 10)
			  {
				move_cnt = 0;
				firstmove = false;
			  }
			  //moveWithSlingshot(0);//ACTION  4
			  //Serial.println(getLength_offset());
			  //delay(1000);
			}
			else
			{
				//Serial.println("after");
				moveWithSlingshot(0);
				//Serial.println(getLength_offset());
				//delay(1000);
			}
		}
	}
	if (status == 'f')
	{
		firstmove = true;
		moveBack();
		status = 'w';
	}
}

void special_act()
{
	Mouse.click();
}
