
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define W 128
#define H 64
#define BTN_LEFT 18
#define BTN_RIGHT 19

Adafruit_SSD1306 display(W, H, &Wire, -1);

int playerX = 64;
int score = 0;
int lives = 3;
int enemySpeed = 1;

bool started = false;
bool gameOver = false;

struct Bullet {
  int x, y;
  bool active;
};

struct Enemy {
  int x, y;
  bool active;
};

Bullet bullets[3];
Enemy enemies[5];

unsigned long lastShot = 0;
unsigned long lastEnemy = 0;
unsigned long lastFrame = 0;

void resetGame() {
  playerX = 64;
  score = 0;
  lives = 3;
  enemySpeed = 1;

  for (int i = 0; i < 3; i++)
    bullets[i].active = false;

  for (int i = 0; i < 5; i++)
    enemies[i].active = false;

  lastShot = millis();
  lastEnemy = millis();
  started = true;
  gameOver = false;
}

void drawPlayer() {
  display.fillTriangle(
    playerX, 44,
    playerX - 6, 55,
    playerX + 6, 55,
    SSD1306_WHITE
  );
  display.fillRect(
    playerX - 2, 50, 4, 6,
    SSD1306_WHITE
  );
}

void drawEnemy(int x, int y) {
  display.fillRect(x, y, 9, 6, SSD1306_WHITE);
  display.drawPixel(x + 2, y + 2, SSD1306_BLACK);
  display.drawPixel(x + 6, y + 2, SSD1306_BLACK);
  display.drawPixel(x - 1, y + 6, SSD1306_WHITE);
  display.drawPixel(x + 9, y + 6, SSD1306_WHITE);
}

void drawScene() {
  display.clearDisplay();

  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.print("SCORE:");
  display.print(score);

  display.setCursor(88, 0);
  display.print("LIFE:");
  display.print(lives);

  display.drawPixel(15, 20, SSD1306_WHITE);
  display.drawPixel(40, 30, SSD1306_WHITE);
  display.drawPixel(100, 15, SSD1306_WHITE);

  drawPlayer();

  for (int i = 0; i < 3; i++) {
    if (bullets[i].active)
      display.drawFastVLine(
        bullets[i].x, bullets[i].y, 4,
        SSD1306_WHITE
      );
  }

  for (int i = 0; i < 5; i++) {
    if (enemies[i].active)
      drawEnemy(enemies[i].x, enemies[i].y);
  }

  if (!started) {
    display.fillRect(8, 18, 112, 30, SSD1306_BLACK);
    display.setCursor(24, 22);
    display.print("SPACE SHOOTER");
    display.setCursor(25, 36);
    display.print("Press any button");
  }

  if (gameOver) {
    display.fillRect(8, 18, 112, 30, SSD1306_BLACK);
    display.drawRect(8, 18, 112, 30, SSD1306_WHITE);
    display.setCursor(31, 22);
    display.print("GAME OVER");
    display.setCursor(25, 35);
    display.print("Score:");
    display.print(score);
    display.setCursor(18, 54);
    display.print("Press to Retry");
  }

  display.display();
}

void setup() {
  pinMode(BTN_LEFT, INPUT_PULLUP);
  pinMode(BTN_RIGHT, INPUT_PULLUP);

  Wire.begin(21, 22);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
    while (true) {}

  randomSeed(micros());
  drawScene();
}

void loop() {
  bool left = digitalRead(BTN_LEFT) == LOW;
  bool right = digitalRead(BTN_RIGHT) == LOW;

  if (!started || gameOver) {
    if (left || right) {
      delay(180);
      resetGame();
    }
    drawScene();
    delay(20);
    return;
  }

  // Move left and right while holding buttons
  if (left) playerX -= 2;
  if (right) playerX += 2;

  playerX = constrain(playerX, 7, 121);

  // Automatic continuous shooting
  if (millis() - lastShot >= 180) {
    for (int i = 0; i < 3; i++) {
      if (!bullets[i].active) {
        bullets[i].x = playerX;
        bullets[i].y = 43;
        bullets[i].active = true;
        lastShot = millis();
        break;
      }
    }
  }

  // Move bullets upward
  for (int i = 0; i < 3; i++) {
    if (bullets[i].active) {
      bullets[i].y -= 3;
      if (bullets[i].y < 0)
        bullets[i].active = false;
    }
  }

  // Spawn enemies
  if (millis() - lastEnemy >= 850) {
    for (int i = 0; i < 5; i++) {
      if (!enemies[i].active) {
        enemies[i].x = random(3, 116);
        enemies[i].y = 8;
        enemies[i].active = true;
        lastEnemy = millis();
        break;
      }
    }
  }

  // Move enemies and detect collisions
  for (int i = 0; i < 5; i++) {
    if (!enemies[i].active) continue;

    enemies[i].y += enemySpeed;

    if (enemies[i].y > 58) {
      enemies[i].active = false;
      lives--;
      if (lives <= 0) gameOver = true;
    }

    for (int j = 0; j < 3; j++) {
      if (enemies[i].active && bullets[j].active &&
          bullets[j].x >= enemies[i].x - 2 &&
          bullets[j].x <= enemies[i].x + 10 &&
          bullets[j].y <= enemies[i].y + 7 &&
          bullets[j].y >= enemies[i].y - 3) {
        enemies[i].active = false;
        bullets[j].active = false;
        score++;

        if (score % 5 == 0) enemySpeed++;
      }
    }

    // Enemy hits player
    if (enemies[i].active &&
        enemies[i].y >= 43 &&
        enemies[i].x <= playerX + 6 &&
        enemies[i].x + 9 >= playerX - 6) {
      enemies[i].active = false;
      lives--;
      if (lives <= 0) gameOver = true;
    }
  }

  drawScene();
  delay(25);
}
