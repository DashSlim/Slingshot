/*
 * measure.cpp
 *
 *  Created on: Apr 2, 2013
 *      Author: dash
 */


#include <Arduino.h>
#include "threshold.h"
#include <HX711.h>

#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
#include "printf.h"

HX711 hx1(4, 5);
HX711 hx2(6, 7);

//---------------------------------------------------------------wireless
// Set up nRF24L01 radio on SPI bus plus pins 9 & 10 

RF24 radio(8,9);

//
// Topology
//

// Radio pipe addresses for the 2 nodes to communicate.
const uint64_t pipes[2] = { 0xF0F0F0F0E1LL, 0xF0F0F0F0D2LL };

//
// Role management
//
// Set up role.  This sketch uses the same software for all the nodes
// in this system.  Doing so greatly simplifies testing.  
//

// The various roles supported by this sketch
typedef enum { role_ping_out = 1, role_pong_back } role_e;

// The debug-friendly names of those roles
const char* role_friendly_name[] = { "invalid", "Ping out", "Pong back"};

// The role of the current running sketch
role_e role = role_pong_back;
//---------------------------------------------------------------wireless

//var below is for angle measure
double xyz[3] = {0, 0, 0};
//var above is for angle measure

double force = 0;
double force_last = 0;
double length = 0;
double length_offset = 0;
double angle = 0;


void getForce();
double force2length(double force);

int sample[13] = {0,0,1,2,3,4,5,6,7,8,9,0,0};

int sampleA = 0;
int sampleF = 0;

//HX711 hx(1,0);

bool suddenFall()
{
	/*force = getForce();
        Serial.print(force_last);
        Serial.print("-------");
        Serial.println(force);
	*/
        if(force_last - force > suddenForce)
	{
		return true;
	}
	else
	{
		return false;
	}
}

bool tight()
{
	getForce();
	if(force > tightenForce*2)
	{
                length_offset = force2length(force);
		return true;
	}
	else
	{
		return false;
	}
}

double getLength()
{
	length = force2length(force);
	return length;
}

double force2length(double force)
{
        double ratio = 200/maxSingleForce;
	return force*ratio;
}

void getAngle()
{
  //
  // Ping out role.  Repeatedly send the current time
  //
  
  int valid_count = 0;
  int valid_limit = 5;
  long got_angle = 0;
  long valid_angle = 0;
  
  while(valid_count < valid_limit)
  {
    if (role == role_ping_out)
    {
      // First, stop listening so we can talk.
      radio.stopListening();
  
      // Take the time, and send it.  This will block until complete
      unsigned long time = millis();
      printf("Now sending %lu...",time);
      bool ok = radio.write( &time, sizeof(unsigned long) );
      
      if (ok)
        printf("ok...");
      else
        printf("failed.\n\r");
  
      // Now, continue listening
      radio.startListening();
  
      // Wait here until we get a response, or timeout (250ms)
      unsigned long started_waiting_at = millis();
      bool timeout = false;
      while ( ! radio.available() && ! timeout )
        if (millis() - started_waiting_at > 200 )
          timeout = true;
  
      // Describe the results
      if ( timeout )
      {
        printf("Failed, response timed out.\n\r");
      }
      else
      {
        // Grab the response, compare, and send to debugging spew
        unsigned long got_time;
        radio.read( &got_time, sizeof(unsigned long) );
  
        // Spew it
        printf("Got response %lu, round-trip delay: %lu\n\r",got_time,millis()-got_time);
      }
  
      // Try again 1s later
      //delay(1000);
    }
  
    //
    // Pong back role.  Receive each packet, dump it out, and send it back
    //
  
    if ( role == role_pong_back )
    {
      // if there is data ready
      if ( radio.available() )
      {
        // Dump the payloads until we've gotten everything
        
        bool done = false;
        while (!done)
        {
          // Fetch the payload, and see if this was the last one.
          done = radio.read( &got_angle, sizeof(long) );
  
          // Spew it
          //printf("Got payload %l...",got_time);
        //if the fetched angel is valid, sum to valid_angle  
        if(got_angle > -10000 && got_angle < 10000)
        {
          valid_count++;
          valid_angle += got_angle;
        }
  	// Delay just a little bit to let the other unit
  	// make the transition to receiver
  	//delay(20);
        }
  
        // First, stop listening so we can talk
        radio.stopListening();
  
        // Send the final one back.
        radio.write( &got_angle, sizeof(long) );
        //printf("Sent response.\n\r");
  
        // Now, resume listening so we catch the next packets.
        radio.startListening();
      }
    }
  
    //
    // Change roles
    //
  
    if ( Serial.available() )
    {
      char c = toupper(Serial.read());
      if ( c == 'T' && role == role_pong_back )
      {
        printf("*** CHANGING TO TRANSMIT ROLE -- PRESS 'R' TO SWITCH BACK\n\r");
  
        // Become the primary transmitter (ping out)
        role = role_ping_out;
        radio.openWritingPipe(pipes[0]);
        radio.openReadingPipe(1,pipes[1]);
      }
      else if ( c == 'R' && role == role_ping_out )
      {
        printf("*** CHANGING TO RECEIVE ROLE -- PRESS 'T' TO SWITCH BACK\n\r");
        
        // Become the primary receiver (pong back)
        role = role_pong_back;
        radio.openWritingPipe(pipes[1]);
        radio.openReadingPipe(1,pipes[0]);
      }
    }
  }
  angle = float(valid_angle)/valid_limit/100;
  //Serial.print("angle:");
  //Serial.println(angle);
}

void getForce()
{
	int sample = 10;
	double sum1 = 0;
	double sum2 = 0;
  
	while(true)
	{
		for (int i = 0; i < sample; i++)
		{
			sum1 += hx1.bias_read();
			sum2 += hx2.bias_read();
		}
		sum1 = sum1/sample;
		sum2 = sum2/sample;
		//remove incorrect data
		if(sum1 > minSingleForce && sum2 > minSingleForce && sum1 < maxSingleForce && sum2 < maxSingleForce)
			break;
                else
                {
                        Serial.print("invalid force: ");
                        Serial.print(sum1);
                        Serial.print("  ");
                        Serial.println(sum2);
                }
	}

	force_last = force;
        if(sum1 < 0 || sum2 < 0)
          force = 0;
        else
          force = (sum1 + sum2);
        //Serial.print("force:");
        //Serial.print(force);
}

void initHX711()
{
	hx1.set_offset(35000);
	 hx2.set_offset(39000);
}

void initnRF24L01()
{
  boolean initdone = false;
  //
  // Print preamble
  //
  delay(4000);
  Serial.println("nrf24 is initing");
  printf_begin();
  printf("\n\rRF24/examples/GettingStarted/\n\r");
  printf("ROLE: %s\n\r",role_friendly_name[role]);
  printf("*** PRESS 'T' to begin transmitting to the other node\n\r");

  //
  // Setup and configure rf radio
  //

  radio.begin();

  // optionally, increase the delay between retries & # of retries
  radio.setRetries(15,15);

  // optionally, reduce the payload size.  seems to
  // improve reliability
  //radio.setPayloadSize(8);

  //
  // Open pipes to other nodes for communication
  //

  // This simple sketch opens two pipes for these two nodes to communicate
  // back and forth.
  // Open 'our' pipe for writing
  // Open the 'other' pipe for reading, in position #1 (we can have up to 5 pipes open for reading)

  //if ( role == role_ping_out )
  {
    //radio.openWritingPipe(pipes[0]);
    radio.openReadingPipe(1,pipes[1]);
  }
  //else
  {
    //radio.openWritingPipe(pipes[1]);
    //radio.openReadingPipe(1,pipes[0]);
  }

  //
  // Start listening
  //

  radio.startListening();

  //
  // Dump the configuration of the rf unit for debugging
  //

  radio.printDetails();
  
  while(!initdone)
  {
        //
    // Ping out role.  Repeatedly send the current time
    //
  
    if (role == role_ping_out)
    {
      // First, stop listening so we can talk.
      radio.stopListening();
  
      // Take the time, and send it.  This will block until complete
      unsigned long time = millis();
      printf("Now sending %lu...",time);
      bool ok = radio.write( &time, sizeof(unsigned long) );
      
      if (ok)
        printf("ok...");
      else
        printf("failed.\n\r");
  
      // Now, continue listening
      radio.startListening();
  
      // Wait here until we get a response, or timeout (250ms)
      unsigned long started_waiting_at = millis();
      bool timeout = false;
      while ( ! radio.available() && ! timeout )
        if (millis() - started_waiting_at > 200 )
          timeout = true;
  
      // Describe the results
      if ( timeout )
      {
        printf("Failed, response timed out.\n\r");
      }
      else
      {
        // Grab the response, compare, and send to debugging spew
        unsigned long got_time;
        radio.read( &got_time, sizeof(unsigned long) );
  
        // Spew it
        printf("Got response %lu, round-trip delay: %lu\n\r",got_time,millis()-got_time);
      }
  
      // Try again 1s later
      delay(1000);
    }
  
    //
    // Pong back role.  Receive each packet, dump it out, and send it back
    //
  
    if ( role == role_pong_back )
    {
      // if there is data ready
      if ( radio.available() )
      {
        // Dump the payloads until we've gotten everything
        long got_time;
        bool done = false;
        while (!done)
        {
          // Fetch the payload, and see if this was the last one.
          done = radio.read( &got_time, sizeof(long) );
  
          // Spew it
          //printf("Got payload %l...",got_time);
          Serial.println(got_time);
          Serial.println("Raedy to gather angle");
          initdone = true;  
  	// Delay just a little bit to let the other unit
  	// make the transition to receiver
  	delay(20);
        }
  
        // First, stop listening so we can talk
        radio.stopListening();
  
        // Send the final one back.
        radio.write( &got_time, sizeof(long) );
        //printf("Sent response.\n\r");
  
        // Now, resume listening so we catch the next packets.
        radio.startListening();
      }
    }
  
    //
    // Change roles
    //
  
    if ( Serial.available() )
    {
      char c = toupper(Serial.read());
      if ( c == 'T' && role == role_pong_back )
      {
        printf("*** CHANGING TO TRANSMIT ROLE -- PRESS 'R' TO SWITCH BACK\n\r");
  
        // Become the primary transmitter (ping out)
        role = role_ping_out;
        radio.openWritingPipe(pipes[0]);
        radio.openReadingPipe(1,pipes[1]);
      }
      else if ( c == 'R' && role == role_ping_out )
      {
        printf("*** CHANGING TO RECEIVE ROLE -- PRESS 'T' TO SWITCH BACK\n\r");
        
        // Become the primary receiver (pong back)
        role = role_pong_back;
        radio.openWritingPipe(pipes[1]);
        radio.openReadingPipe(1,pipes[0]);
      }
    }
  }
}

double getAngleRad()
{
  return angle*3.1415926/180;
}

double getLength_offset()
{
  return length_offset;
}