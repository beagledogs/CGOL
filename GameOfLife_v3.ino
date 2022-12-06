#include <Wire.h>
#include "Adafruit_LEDBackpack.h"
#include "Adafruit_GFX.h"
#include "Board.h"

#define MATRIXone 0
#define MATRIXtwo 1
#define ANTI_BOUNCE_TIME 10
#define BUTTON_PIN A1               // Change according to the pin the button is connected to
#define NUMBER_OF_ITERATIONS 500 // Change according to the number of iterations desired

typedef enum
{
  RUNNING,
  RUNNING_FOREVER
} SystemStateEnum;

typedef enum
{
  NO_UPDATE,
  OFF,
  PRESSED,
  ON,
  RELEASED
} ButtonStateEnum;

ButtonStateEnum get_button_state(int button_pin)
{
  static ButtonStateEnum sOldButtonState = OFF;
  static size_t sLastTime = 0;

  if (millis() - sLastTime >= ANTI_BOUNCE_TIME)
  {
    sLastTime = millis();
    bool isOn = !digitalRead(button_pin); // If we set the pinMode to INPUT_PULLUP, Button Pressed = 0 Button Not Pressed = 1

    switch (sOldButtonState)
    {
    case OFF:
      if (isOn)
      {
        sOldButtonState = PRESSED;
      }
      break;
    case PRESSED:
      if (isOn)
      {
        sOldButtonState = ON;
      }
      else
      {
        sOldButtonState = RELEASED;
      }
      break;
    case ON:
      if (!isOn)
      {
        sOldButtonState = RELEASED;
      }
      break;
    case RELEASED:
      if (isOn)
      {
        sOldButtonState = PRESSED;
      }
      else
      {
        sOldButtonState = OFF;
      }
    }
    return sOldButtonState;
  }
  return NO_UPDATE;
}

Adafruit_8x16matrix matrix[2] = { // Array of Adafruit_8x8matrix objects
    Adafruit_8x16matrix(), Adafruit_8x16matrix()};

void display_board(const Board &board)
{
  for (size_t y = 0; y < board.size(); y++)
  {
    for (size_t x = 0; x < board.size(); x++)
    {
      if (board.get(x, y))
      {
        matrix[MATRIXone].drawPixel(x, y, LED_ON);
        matrix[MATRIXtwo].drawPixel(x, y - 8, LED_ON);
      }
      else
      {
        matrix[MATRIXone].drawPixel(x, y, LED_OFF);
        matrix[MATRIXtwo].drawPixel(x, y - 8, LED_OFF);
      }
    }
  }
  matrix[MATRIXone].writeDisplay();
  matrix[MATRIXtwo].writeDisplay();
}

void next(const Board &src, Board &dest)
{
  for (size_t y = 0; y < src.size(); y++)
  {
    for (size_t x = 0; x < src.size(); x++)
    {
      int neighbors = src.getModulus(x - 1, y + 1) + src.getModulus(x, y + 1) + src.getModulus(x + 1, y + 1) +
                      src.getModulus(x - 1, y) + src.getModulus(x + 1, y) +
                      src.getModulus(x - 1, y - 1) + src.getModulus(x, y - 1) + src.getModulus(x + 1, y - 1);
      bool live = true;

      if (src.get(x, y))
      {
        if (neighbors < 2 || 3 < neighbors)
        {
          live = false;
        }
      }
      else
      {
        if (neighbors != 3)
        {
          live = false;
        }
      }

      dest.set(x, y, live);
    }
  }
}

void display_iter(const Board &src, Board &dest)
{
  display_board(src);
  delay(50);
  next(src, dest);
}

void randomize(Board &board)
{
  for (size_t y = 0; y < board.size(); y++)
  {
    for (size_t x = 0; x < board.size(); x++)
    {
      board.set(x, y, random(2));
    }
  }
}

Board boardeven;
Board boardodd;
bool even = true;

void setup()
{
  matrix[MATRIXone].begin(0x70); // pass in the address
  matrix[MATRIXtwo].begin(0x71); // pass in the address
  matrix[MATRIXone].setRotation(1);
  matrix[MATRIXtwo].setRotation(1);
  matrix[MATRIXone].setBrightness(1);
  matrix[MATRIXtwo].setBrightness(1);
  randomSeed(analogRead(3));

  randomize(boardeven);
  pinMode(BUTTON_PIN, INPUT_PULLUP); // connect one side of the switch to the arduino pin and the other to GND
  //  boardeven.set(1, 0, true);
  //  boardeven.set(2, 1, true);
  //  boardeven.set(0, 2, true);
  //  boardeven.set(1, 2, true);
  //  boardeven.set(2, 2, true);
  even = true;
}

void loop()
{

  static size_t iterations = 0;
  static SystemStateEnum systemState = RUNNING;
  bool isButtonPressed = get_button_state(BUTTON_PIN) == PRESSED;

  if (isButtonPressed)
  {
    systemState = systemState == RUNNING ? RUNNING_FOREVER : RUNNING;
    iterations = 0;
  }
  if (systemState == RUNNING)
  {
    iterations++;
  }

  if (even)
  {
    display_iter(boardeven, boardodd);
  }
  else
  {
    display_iter(boardodd, boardeven);
  }
  even = !even;
  if (boardeven == boardodd || iterations == NUMBER_OF_ITERATIONS)
  {
    iterations = 0;
    delay(1000);
    randomize(boardeven);
    even = true;
  }
}
