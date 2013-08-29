/*
 * measure.h
 *
 *  Created on: Apr 2, 2013
 *      Author: dash
 */

#ifndef MEASURE_H_
#define MEASURE_H_

void initHX711();
void initnRF24L01();

void getForce();
void getAngle();

double getLength();
double getLength_offset();
double getAngleRad();

bool suddenFall();
bool tight();


#endif /* MEASURE_H_ */
