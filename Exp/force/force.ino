#include <HX711.h>

HX711 hx1(4, 5);
HX711 hx2(6, 7);

double force = 0;
double force_last = 0;

void setup() {
  Serial.begin(9600);
  hx1.set_offset(35000);
  hx2.set_offset(39000);
}

void loop()
{
  double sum1 = 0;
  double sum2 = 0;
  int sample = 10;
  
  for (int i = 0; i < sample; i++)
    sum2 += hx2.bias_read();
  for (int i = 0; i < sample; i++)
    sum1 += hx1.bias_read();
  force_last = force;
  force = (sum2+sum1)/sample;
  Serial.print("force: ");
  Serial.print(force);
  Serial.print(" last - force: ");
  Serial.println(force_last - force);
  delay(100);
}
