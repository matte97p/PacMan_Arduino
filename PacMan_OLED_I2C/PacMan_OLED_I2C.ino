#include <OLED_I2C.h>
#include <Tone.h>

// Define OLED display settings
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
OLED myOLED(SDA, SCL);
extern uint8_t SmallFont[];

// Define the grid and cell sizes
#define GRID_WIDTH 16
#define GRID_HEIGHT 8
#define CELL_WIDTH (SCREEN_WIDTH / GRID_WIDTH)
#define CELL_HEIGHT (SCREEN_HEIGHT / GRID_HEIGHT)

// Joystick PS2
#define VRX A0
#define VRY A1
#define SW 2

// Buzzer
#define buzzerPin 8
Tone buzzer;

// Grid
// Bit 0-3: Object presence (0-15 objects, if more than 15, wall is assumed)
// Bit 4: Wall presence (1 if wall, 0 if empty)
// 0x10 wall - 0x01 object - 0x00 empty
uint8_t grid[GRID_WIDTH][GRID_HEIGHT];
uint8_t totalObjects;

// Directions are like a clock
// UP 0 - RIGHT 1 - DOWN 2 - LEFT 3

// Pac-Man
uint8_t pacManX;
uint8_t pacManY;
bool animationActive;
uint8_t pacManDirection;
#define updateInterval 200
#define VRX_THRESHOLD_LOW  400
#define VRX_THRESHOLD_HIGH 600
#define VRY_THRESHOLD_LOW  400
#define VRY_THRESHOLD_HIGH 600

// Ghost
const uint8_t numGhosts = 2;
uint8_t ghostX[numGhosts] = { GRID_WIDTH - 2, GRID_WIDTH - 2 };
uint8_t ghostY[numGhosts] = { 1, GRID_HEIGHT - 2 };
unsigned long lastGhostMove = 0;
#define ghostSpeed 400

// Checks
unsigned long lastUpdate = 0;
uint8_t currLevel = 1;
uint8_t score = 0;
bool gameOver = false;
bool win = false;

void setup() {
  // Initializes serial communication for debugging
  Serial.begin(9600);

  // Pin setup
  pinMode(VRX, INPUT);
  pinMode(VRY, INPUT);
  pinMode(SW, INPUT_PULLUP);

  // Initialize display
  if(!myOLED.begin(SSD1306_128X64)) {
    Serial.println(F("Error initializing the OLED display!"));
    // Infinite loop to indicate a fatal error
    while (1);
  }
  myOLED.setFont(SmallFont);
  myOLED.clrScr();

  // Start buzzer
  buzzer.begin(buzzerPin);

  randomSeed(analogRead(0));

  // Initialize the grid
  initializeGrid();
}

// Function to initialize the grid with objects and walls
void initializeGrid() {
  initializeParams();
  
  // Set the walls according to the level and the dots to eat in the empty cells
  for (uint8_t x = 0; x < GRID_WIDTH; x++) {
    for (uint8_t y = 0; y < GRID_HEIGHT; y++) {
      // External walls
      if (x == 0 || x == GRID_WIDTH - 1 || y == 0 || y == GRID_HEIGHT - 1) {
        grid[x][y] = 0x10;
      } else {
        bool isWall = false;
        switch(currLevel){
          case 0:
            grid[x][y] = 0x01;
            totalObjects++;
            break;
          case 1:
            isWall = level1(x, y);
            break;
          case 2:
            isWall = level2(x, y);
            break;
          case 3:
            isWall = level3(x, y);
            break;
        }
        if(isWall) {
          grid[x][y] = 0x10;
        } else {
          grid[x][y] = 0x01;
          totalObjects++;
        }
      }
    }
  }

  // Remove the object from Pac-Man's starting position
  grid[pacManX][pacManY] = 0x00;
  totalObjects--;
}

void initializeParams() {
  // Walls & Objects
  totalObjects = 0;

  // Pac-Man
  pacManX = 1;
  pacManY = 1;
  animationActive = true;
  pacManDirection = 1;
  
  // Ghost
  lastGhostMove = 0;
  for (uint8_t i = 0; i < numGhosts; i++) {
    ghostX[i] = GRID_WIDTH - 2;
    if(i == 0) ghostY[i] = 1;
    else  ghostY[i] = GRID_HEIGHT - 2;
  }
  
  // Checks
  lastUpdate = 0;
  score = 0;
  gameOver = false;
  win = false;
}

bool level1(uint8_t x, uint8_t y) {
  //   { '#', '#', '#', '#', '#', '#', '#', '#' },
  //   { '#', '.', '.', '.', '.', '.', '.', '#' },
  //   { '#', '.', '#', '#', '#', '#', '.', '#' },
  //   { '#', '.', '#', '.', '.', '#', '.', '#' },

  //   { '#', '.', '#', '.', '.', '#', '.', '#' },
  //   { '#', '.', '#', '.', '.', '#', '.', '#' },
  //   { '#', '.', '.', '.', '.', '.', '.', '#' },
  //   { '#', '.', '#', '#', '#', '#', '.', '#' },

  //   { '#', '.', '.', '#', '.', '.', '.', '#' },
  //   { '#', '.', '.', '#', '.', '.', '.', '#' },
  //   { '#', '#', '.', '.', '.', '.', '#', '#' },
  //   { '#', '.', '.', '#', '#', '.', '.', '#' },

  //   { '#', '.', '.', '.', '.', '.', '.', '#' },
  //   { '#', '.', '#', '#', '#', '#', '.', '#' },
  //   { '#', '.', '.', '.', '.', '.', '.', '#' },
  //   { '#', '#', '#', '#', '#', '#', '#', '#' }
  switch (x) {
    case 2:
      return (y == 2 || y == 3 || y == 4 || y == 5);
    case 3:
      return (y == 2 || y == 5);
    case 4:
      return (y == 2 || y == 5);
    case 5:
      return (y == 2 || y == 5);
    case 7:
      return (y == 2 || y == 3 || y == 4 || y == 5);
    case 8:
      return (y == 3);
    case 9:
      return (y == 3);
    case 10:
      return (y == 1 || y == 6);
    case 11:
      return (y == 3 || y == 4);
    case 13:
      return (y == 2 || y == 3 || y == 4 || y == 5);
    default:
      return false;
  }
}

bool level2(uint8_t x, uint8_t y) {
  //   { '#', '#', '#', '#', '#', '#', '#', '#' },
  //   { '#', '.', '.', '.', '.', '.', '.', '#' },
  //   { '#', '.', '#', '#', '.', '#', '.', '#' },
  //   { '#', '.', '.', '#', '.', '#', '.', '#' },
  //   { '#', '#', '.', '#', '.', '#', '.', '#' },
  //   { '#', '.', '.', '#', '.', '.', '.', '#' },
  //   { '#', '.', '#', '#', '#', '#', '.', '#' },
  //   { '#', '.', '.', '.', '.', '.', '.', '#' },
  //   { '#', '.', '#', '#', '.', '#', '.', '#' },
  //   { '#', '.', '.', '.', '.', '#', '.', '#' },
  //   { '#', '.', '#', '#', '.', '#', '.', '#' },
  //   { '#', '.', '.', '#', '.', '#', '.', '#' },
  //   { '#', '#', '.', '#', '.', '#', '.', '#' },
  //   { '#', '.', '.', '.', '.', '#', '.', '#' },
  //   { '#', '.', '.', '.', '.', '.', '.', '#' },
  //   { '#', '#', '#', '#', '#', '#', '#', '#' }
  switch (x) {
    case 2:
      return (y == 2 || y == 3 || y == 5);
    case 3:
      return (y == 3 || y == 5);
    case 4:
      return (y == 1 || y == 3 || y == 5);
    case 5:
      return (y == 3);
    case 6:
      return (y == 2 || y == 3 || y == 4 || y == 5);
    case 8:
      return (y == 2 || y == 3 || y == 5);
    case 9:
      return (y == 5);
    case 10:
      return (y == 2 || y == 3 || y == 5);
    case 11:
      return (y == 3 || y == 5);
    case 12:
      return (y == 1 || y == 3 || y == 5);
    case 13:
      return (y == 5);
    default:
      return false;
  }
}

bool level3(uint8_t x, uint8_t y) {
  //   { '#', '#', '#', '#', '#', '#', '#', '#' },
  //   { '#', '.', '.', '.', '.', '.', '.', '#' },
  //   { '#', '.', '#', '#', '.', '#', '.', '#' },
  //   { '#', '.', '.', '#', '.', '#', '.', '#' },
  //   { '#', '#', '.', '#', '.', '#', '.', '#' },
  //   { '#', '.', '.', '#', '.', '.', '.', '#' },
  //   { '#', '.', '#', '#', '#', '#', '.', '#' },
  //   { '#', '.', '.', '.', '.', '.', '.', '#' },
  //   { '#', '.', '#', '#', '.', '#', '.', '#' },
  //   { '#', '.', '#', '#', '.', '#', '.', '#' },
  //   { '#', '.', '.', '.', '.', '#', '.', '#' },
  //   { '#', '.', '#', '#', '.', '#', '.', '#' },
  //   { '#', '.', '#', '.', '.', '#', '.', '#' },
  //   { '#', '.', '#', '.', '#', '#', '.', '#' },
  //   { '#', '.', '.', '.', '.', '.', '.', '#' },
  //   { '#', '#', '#', '#', '#', '#', '#', '#' }
  
  switch (x) {
    case 2:
      return (y == 2 || y == 3 || y == 5);
    case 3:
      return (y == 3 || y == 5);
    case 4:
      return (y == 1 || y == 3 || y == 5);
    case 5:
      return (y == 3);
    case 6:
      return (y == 2 || y == 3 || y == 4 || y == 5);
    case 8:
      return (y == 2 || y == 3 || y == 5);
    case 9:
      return (y == 2 || y == 3 || y == 5);
    case 10:
      return (y == 5);
    case 11:
      return (y == 2 || y == 3 || y == 5);
    case 12:
      return (y == 2 || y == 5);
    case 13:
      return (y == 2 || y == 4 || y == 5);
    default:
      return false;
  }
}

// Function for drawing the grid, walls, objects, Pac-Man and ghosts
void drawGrid() {
  myOLED.clrScr();
  drawWallsAndObjects();
  drawPacMan();
  drawGhost();
  myOLED.update();

  checkWin();
  checkCollision();
}

// Function for drawing walls and objects
void drawWallsAndObjects() {
  for (uint8_t x = 0; x < GRID_WIDTH; x++) {
    for (uint8_t y = 0; y < GRID_HEIGHT; y++) {
      if (grid[x][y] & 0x10) {
        myOLED.drawRect(x * CELL_WIDTH, y * CELL_HEIGHT, x * CELL_WIDTH + CELL_WIDTH, y * CELL_HEIGHT + CELL_HEIGHT);
      } else if (grid[x][y] & 0x01) {
        myOLED.drawCircle(x * 8 + 4, y * 8 + 4, 1.5);
      }
    }
  }
}

// Function for drawing Pac-Man
void drawPacMan() {
  uint8_t centerX = pacManX * CELL_WIDTH + CELL_WIDTH / 2;
  uint8_t centerY = pacManY * CELL_HEIGHT + CELL_HEIGHT / 2;
  uint8_t radius = CELL_WIDTH / 3;
  uint8_t mouth_radius = radius + 1;
  uint8_t angle_start = 45;
  int angle_end = 315;

  if (animationActive) {
    for (uint8_t r = 0; r <= radius; r++) {
      for (int angle = angle_start; angle <= angle_end; angle++) {
        uint8_t mouth_x = centerX + r * cos(angle * PI / 180);
        uint8_t mouth_y = centerY + r * sin(angle * PI / 180);
        myOLED.setPixel(mouth_x, mouth_y);
      }
    }
  } else {
    myOLED.drawCircle(centerX, centerY, radius);
    for (uint8_t y = -radius; y <= radius; y++) {
        for (uint8_t x = -radius; x <= radius; x++) {
            if (x * x + y * y <= radius * radius) {
                myOLED.setPixel(centerX + x, centerY + y);
            }
        }
    }
  }
}
// void drawPacMan() {
//   uint8_t CELL_WIDTH = SCREEN_WIDTH / GRID_WIDTH;
//   uint8_t CELL_HEIGHT = SCREEN_HEIGHT / GRID_HEIGHT;
//   uint8_t centerX = pacManX * CELL_WIDTH + CELL_WIDTH / 2;
//   uint8_t centerY = pacManY * CELL_HEIGHT + CELL_HEIGHT / 2;
//   uint8_t radius = CELL_WIDTH / 4;

//   // Draw Pac-Man's body
//   for (uint8_t y = -radius; y <= radius; y++) {
//     for (uint8_t x = -radius; x <= radius; x++) {
//       if (x * x + y * y <= radius * radius) {
//         myOLED.setPixel(centerX + x, centerY + y);  // Disegna i pixel del cerchio di Pac-Man
//       }
//     }
//   }

//   // // Disegna la bocca di Pac-Man (triangolo)
//   // for (uint8_t y = -2; y <= 2; y++) {
//   //   for (uint8_t x = 0; x <= radius; x++) {
//   //     if (x * x + y * y <= radius * radius && x >= abs(y)) {
//   //       myOLED.clrPixel(centerX + x, centerY + y);  // Disegna i pixel della bocca di Pac-Man
//   //     }
//   //   }
//   // }

//   // Draw the mouth opening
//   uint8_t startAngle, endAngle;
//   if (animationActive) {
//     uint8_t angleOffset = 22;

//     switch (pacManDirection) {
//       case DIR_RIGHT:
//         startAngle = angleOffset;
//         endAngle = 360 - angleOffset;
//         break;
//       case DIR_LEFT:
//         startAngle = 180 + angleOffset;
//         endAngle = 180 - angleOffset;
//         break;
//       case DIR_UP:
//         startAngle = 270 + angleOffset;
//         endAngle = 270 - angleOffset;
//         break;
//       case DIR_DOWN:
//         startAngle = 90 + angleOffset;
//         endAngle = 90 - angleOffset;
//         break;
//     }

//     myOLED.clrLine(centerX, centerY, centerX + radius * cos(radians(startAngle)), centerY - radius * sin(radians(startAngle)));
//     myOLED.clrLine(centerX, centerY, centerX + radius * cos(radians(endAngle)), centerY - radius * sin(radians(endAngle)));
//   }
// }

// Function for drawing ghosts
void drawGhost() {
  for (uint8_t i = 0; i < numGhosts; i++) {
    // Calculate the center of the cell where the ghost is located
    uint8_t centerX = ghostX[i] * CELL_WIDTH + CELL_WIDTH / 2;
    uint8_t centerY = ghostY[i] * CELL_HEIGHT + CELL_HEIGHT / 2;

    // Radius and body size of the ghost
    uint8_t bodyWidth = CELL_WIDTH * 2 / 3;
    uint8_t bodyHeight = CELL_HEIGHT * 2 / 3;
    
    // Draw the ghost's body
    myOLED.drawRect(centerX - bodyWidth / 2, centerY - bodyHeight / 2,
                    centerX + bodyWidth / 2 - 1, centerY + bodyHeight / 2 - 1);

    // Draw the ghost's head
    uint8_t headRadius = bodyWidth / 2;
    myOLED.clrCircle(centerX, centerY - bodyHeight / 2 - headRadius / 2, headRadius);

    // Size and position of the eyes
    uint8_t eyeSize = bodyWidth / 8;
    uint8_t eyeOffsetX = bodyWidth / 4;
    uint8_t eyeOffsetY = -bodyHeight / 4;

    // Draw the ghost's eyes
    if (animationActive) {
      myOLED.clrCircle(centerX - eyeOffsetX, centerY - bodyHeight / 2 + eyeOffsetY, eyeSize); // Occhio sinistro
      myOLED.clrCircle(centerX + eyeOffsetX, centerY - bodyHeight / 2 + eyeOffsetY, eyeSize); // Occhio destro
    }
    // if (animationActive) {
    //   myOLED.drawLine(x - 2, y - 2, x - 2, y - 2); // Occhio sinistro
    //   myOLED.drawLine(x + 2, y - 2, x + 2, y - 2); // Occhio destro
    // } else {
    //   myOLED.drawLine(x - 3, y - 2, x - 1, y - 2); // Occhio sinistro
    //   myOLED.drawLine(x + 1, y - 2, x + 3, y - 2); // Occhio destro
    // }
  }
}

// Function for update Pac-Man position
void movePacMan() {
  uint8_t newPx = pacManX;
  uint8_t newPy = pacManY;

  int vrxValue = analogRead(VRX);
  int vryValue = analogRead(VRY);

  // Controls the preferred axis of movement (horizontal or vertical priority)
  if (abs(vrxValue - 512) > abs(vryValue - 512)) {
    // Priority movement on the horizontal axis
    if (vrxValue < VRX_THRESHOLD_LOW && pacManX > 0 && !(grid[pacManX - 1][pacManY] & 0x10)) {
      newPx--;
      pacManDirection = 3;  // Left
    } else if (vrxValue > VRX_THRESHOLD_HIGH && pacManX < GRID_WIDTH - 1 && !(grid[pacManX + 1][pacManY] & 0x10)) {
      newPx++;
      pacManDirection = 1;  // Right
    }
  } else {
    // Priority movement on the vertical axis
    if (vryValue < VRY_THRESHOLD_LOW && pacManY > 0 && !(grid[pacManX][pacManY - 1] & 0x10)) {
      newPy--;
      pacManDirection = 0;  // Up
    } else if (vryValue > VRY_THRESHOLD_HIGH && pacManY < GRID_HEIGHT - 1 && !(grid[pacManX][pacManY + 1] & 0x10)) {
      newPy++;
      pacManDirection = 2;  // Down
    }
  }

  // Update Pac-Man's position and animation
  if (newPx != pacManX || newPy != pacManY) {
    pacManX = newPx;
    pacManY = newPy;
    animationActive = !animationActive;

    // Check to see if Pac-Man has picked up an object
    if (grid[pacManX][pacManY] & 0x01) {
      grid[pacManX][pacManY] &= 0x00;
      buzzer.play(NOTE_C5, 100);
      totalObjects--;
      score += 10;
    }
  }
}

// Function for update ghosts postions
void moveGhosts() {
  for (uint8_t i = 0; i < numGhosts; i++) {
    bool directionFound = false;
    
    while (!directionFound) {
      uint8_t direction = random(4);
      uint8_t newX = ghostX[i];
      uint8_t newY = ghostY[i];
      
      switch (direction) {
        case 0:  // Up
          newY--;
          break;
        case 1:  // Down
          newY++;
          break;
        case 2:  // Left
          newX--;
          break;
        case 3:  // Right
          newX++;
          break;
      }
      
      if (newX >= 0 && newX < GRID_WIDTH && newY >= 0 && newY < GRID_HEIGHT && !(grid[newX][newY] & 0x10)) {
        bool occupied = false;
        
        for (uint8_t j = 0; j < numGhosts; j++) {
          if (j != i && ghostX[j] == newX && ghostY[j] == newY) {
            occupied = true;
            break;
          }
        }
        
        if (!occupied) {
          ghostX[i] = newX;
          ghostY[i] = newY;
          directionFound = true;
        }
      }
    }
  }
}

// Function for checking collision with ghosts
void checkCollision() {
  for (uint8_t i = 0; i < numGhosts; i++) {
    if (pacManX == ghostX[i] && pacManY == ghostY[i]) {
      gameOver = true;
      return;
    }
  }
}

// Function to check if Pac-Man has collected all objects
void checkWin() {
  if (totalObjects <= 40) {
    win = true;
    return;
  }
}

void loop() {
  if (!gameOver && !win) {
    unsigned long currentMillis = millis();

    if (currentMillis - lastUpdate >= updateInterval) {
      lastUpdate = currentMillis;
      movePacMan();

      if (currentMillis - lastGhostMove >= ghostSpeed) {
        lastGhostMove = currentMillis;
        moveGhosts();
      }
      
      drawGrid();
    }
  } else {
    handleGameOver();
  }
}

void handleGameOver() {
  delay(500);
  if (win) {
    displayEndMessage("YOU WIN!!!");
    currLevel = (currLevel % 3) + 1;
  } else {
    displayEndMessage("Game Over");
    currLevel = 1;
  }
  playMelody();
  delay(2000);
  initializeGrid();
}

void displayEndMessage(const char* message) {
  myOLED.clrScr();
  myOLED.print(message, CENTER, 30);
  myOLED.print("Score: " + String(score), CENTER, 40);
  myOLED.update();
}

// melody credits https://e-boombots.com/download/professor-laytons-theme/
const int winMelody[] = {
  NOTE_B4, 143, NOTE_B5, 143, NOTE_FS5, 143, NOTE_DS5, 143,
  NOTE_B5, 71, NOTE_FS5, 214, NOTE_DS5, 286, NOTE_C5, 143,
  NOTE_C6, 143, NOTE_G6, 143, NOTE_E6, 143, NOTE_C6, 71,
  NOTE_G6, 214, NOTE_E6, 286, NOTE_B4, 143, NOTE_B5, 143,
  NOTE_FS5, 143, NOTE_DS5, 143, NOTE_B5, 71, NOTE_FS5, 214,
  NOTE_DS5, 286, NOTE_DS5, 71, NOTE_E5, 71, NOTE_F5, 71,
  NOTE_F5, 71, NOTE_FS5, 71, NOTE_G5, 71, NOTE_G5, 71,
  NOTE_GS5, 71, NOTE_A5, 143, NOTE_B5, 286
};
const int loseMelody[] = {
  NOTE_C4, 200, NOTE_A3, 200, NOTE_D4, 400,
  NOTE_G3, 200, NOTE_G3, 200, NOTE_F3, 400,
  NOTE_E3, 400, NOTE_C3, 800
};
void playMelody() {
  int *melody;
  int *noteCount;
  if (win) {
    melody = (int *)malloc(sizeof(winMelody));
    memcpy(melody, winMelody, sizeof(winMelody));
    noteCount = sizeof(winMelody) / sizeof(winMelody[0]) / 2;
  } else {
    melody = (int *)malloc(sizeof(loseMelody));
    memcpy(melody, loseMelody, sizeof(loseMelody));
    noteCount = sizeof(loseMelody) / sizeof(loseMelody[0]) / 2;
  }

  for (uint8_t thisNote = 0; thisNote < noteCount; thisNote++) {
    uint8_t noteDuration = melody[thisNote * 2 + 1];
    if (melody[thisNote * 2] == 0) {
      delay(noteDuration);
    } else {
      buzzer.play(melody[thisNote * 2], noteDuration * 0.9);
      delay(noteDuration);
    }
  }
  
  // Liberazione della memoria allocata
  free(melody);
  free(noteCount);
}