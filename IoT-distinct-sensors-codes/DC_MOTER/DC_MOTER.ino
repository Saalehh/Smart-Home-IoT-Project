//                    DC MOTER                                  //

// Defining the Moters and potentiometre  
int dc1 = 3;
int potentiometre = A1;

// Defining that they are input or output
void setup ()
{
  pinMode (dc1,OUTPUT);
  pinMode (potentiometre, INPUT);
}

// main programming 
void loop ()
{
  analogWrite(dc1,(analogRead(potentiometre)/4));
  delay(10);
}