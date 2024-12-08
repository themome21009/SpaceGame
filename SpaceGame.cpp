#include <GL/glut.h>
#include <cmath>
#include <iostream>
#include <vector>
#include <string>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h" // Include stb_image for texture loading

using namespace std;

// Game states and counters
float shipX = 400.0f, shipY = 50.0f; // Spaceship position
int score = 0;                       // Score for hitting enemies
int bonusCount = 0;                  // Bonus collectibles count
int painCounter = 0;                 // Hits the player has taken
bool gameOver = false;               // Game over flag
bool gameWin = false;                // Game win flag

GLuint floorTexture, spaceshipTexture, enemyTexture; // Textures

// Projectile structure
struct Projectile {
    float x, y;
    bool active;
};
vector<Projectile> playerProjectiles;
vector<Projectile> enemyProjectiles;

// Enemy structure and list
struct Enemy {
    float x, y;
    float speed;
    bool isAlive;
};
vector<Enemy> enemies = {
    {100.0f, 500.0f, 2.0f, true},
    {300.0f, 500.0f, 2.5f, true},
    {500.0f, 500.0f, 3.0f, true},
    {200.0f, 450.0f, 1.5f, true},
    {600.0f, 450.0f, 2.0f, true}
};

// Collectible structure and list
struct Collectible {
    float x, y;
    bool isCollected;
};
vector<Collectible> collectibles = {
    {150.0f, 600.0f, false}, {400.0f, 800.0f, false}, {600.0f, 700.0f, false}
};

// Utility function to draw a circle
void drawCircle(float x, float y, float radius, float red, float green, float blue) {
    glColor3f(red, green, blue);
    glBegin(GL_POLYGON);
    for (int i = 0; i < 360; ++i) {
        float theta = i * 3.14159f / 180.0f;
        glVertex2f(x + radius * cos(theta), y + radius * sin(theta));
    }
    glEnd();
}

// Load textures using stb_image
GLuint loadTexture(const char* filename) {
    int width, height, channels;
    unsigned char* data = stbi_load(filename, &width, &height, &channels, 0);
    if (!data) {
        cout << "Failed to load texture: " << filename << endl;
        exit(1);
    }

    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    GLenum format = (channels == 4) ? GL_RGBA : GL_RGB;
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_image_free(data);
    return texture;
}

// Load all textures
void loadTextures() {
    floorTexture = loadTexture("floor.png");
    spaceshipTexture = loadTexture("spaceship.png");
    enemyTexture = loadTexture("enemy.png");
}

// Draw textured rectangle
void drawTexturedRectangle(float x, float y, float width, float height, GLuint texture) {
    glBindTexture(GL_TEXTURE_2D, texture);
    glEnable(GL_TEXTURE_2D);

    glBegin(GL_QUADS);
    glTexCoord2f(0.0f, 0.0f); glVertex2f(x, y);
    glTexCoord2f(1.0f, 0.0f); glVertex2f(x + width, y);
    glTexCoord2f(1.0f, 1.0f); glVertex2f(x + width, y + height);
    glTexCoord2f(0.0f, 1.0f); glVertex2f(x, y + height);
    glEnd();

    glDisable(GL_TEXTURE_2D);
}

// Check win condition
void checkWinCondition() {
    bool allEnemiesDefeated = true;
    for (const auto& enemy : enemies) {
        if (enemy.isAlive) {
            allEnemiesDefeated = false;
            break;
        }
    }
    if (allEnemiesDefeated && bonusCount >= collectibles.size()) {
        gameWin = true;
    }
}

// Update game logic
void timer(int value) {
    if (gameOver || gameWin) return;

    for (auto& proj : playerProjectiles) {
        if (proj.active) {
            proj.y += 5.0f;
            if (proj.y > 600.0f) proj.active = false;
        }
    }

    for (auto& proj : enemyProjectiles) {
        if (proj.active) {
            proj.y -= 5.0f;
            if (proj.y < 0.0f) proj.active = false;
        }
    }

    for (auto& enemy : enemies) {
        if (enemy.isAlive) {
            enemy.x += enemy.speed;
            if (enemy.x > 800.0f || enemy.x < 0.0f) enemy.speed *= -1;

            if (rand() % 100 < 2) {
                enemyProjectiles.push_back({ enemy.x + 15.0f, enemy.y, true });
            }
        }
    }

    for (auto& proj : playerProjectiles) {
        if (proj.active) {
            for (auto& enemy : enemies) {
                if (enemy.isAlive && proj.x > enemy.x && proj.x < enemy.x + 30.0f &&
                    proj.y > enemy.y && proj.y < enemy.y + 30.0f) {
                    enemy.isAlive = false;
                    proj.active = false;
                    score += 10;
                }
            }
        }
    }

    for (const auto& proj : enemyProjectiles) {
        if (proj.active && proj.x > shipX - 15.0f && proj.x < shipX + 15.0f &&
            proj.y > shipY - 25.0f && proj.y < shipY) {
            painCounter++;
            if (painCounter >= 5) gameOver = true;
        }
    }

    for (auto& collectible : collectibles) {
        if (!collectible.isCollected) {
            collectible.y -= 2.0f;
            if (collectible.y < 0.0f) collectible.y = 600.0f;
            if (collectible.x > shipX - 15.0f && collectible.x < shipX + 15.0f &&
                collectible.y > shipY - 25.0f && collectible.y < shipY) {
                collectible.isCollected = true;
                bonusCount += 1;
            }
        }
    }

    checkWinCondition();
    glutPostRedisplay();
    glutTimerFunc(16, timer, 0);
}

// Display score, bonus, and hits
void displayScore() {
    glColor3f(1.0f, 1.0f, 1.0f);
    glRasterPos2f(10.0f, 570.0f);
    string scoreText = "Score: " + to_string(score);
    for (char c : scoreText) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c);

    glRasterPos2f(700.0f, 570.0f);
    string bonusText = "Bonus: " + to_string(bonusCount);
    for (char c : bonusText) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c);

    glRasterPos2f(350.0f, 570.0f);
    string painText = "Hits: " + to_string(painCounter);
    for (char c : painText) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c);
}

// Render the scene
void display() {
    glClear(GL_COLOR_BUFFER_BIT);

    if (gameOver) {
        glColor3f(1.0f, 0.0f, 0.0f);
        glRasterPos2f(350.0f, 300.0f);
        string message = "You Died!";
        for (char c : message) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c);
    }
    else if (gameWin) {
        glColor3f(0.0f, 1.0f, 0.0f);
        glRasterPos2f(350.0f, 300.0f);
        string message = "You Win!";
        for (char c : message) glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, c);
    }
    else {
        drawTexturedRectangle(0.0f, 0.0f, 800.0f, 600.0f, floorTexture);
        drawTexturedRectangle(shipX - 15.0f, shipY - 25.0f, 30.0f, 30.0f, spaceshipTexture);

        for (const auto& enemy : enemies) {
            if (enemy.isAlive) {
                drawTexturedRectangle(enemy.x, enemy.y, 30.0f, 30.0f, enemyTexture);
            }
        }

        for (const auto& collectible : collectibles) {
            if (!collectible.isCollected) {
                drawCircle(collectible.x, collectible.y, 10.0f, 1.0f, 1.0f, 0.0f);
            }
        }

        for (const auto& proj : playerProjectiles) {
            if (proj.active) {
                drawCircle(proj.x, proj.y, 5.0f, 1.0f, 1.0f, 1.0f);
            }
        }
        for (const auto& proj : enemyProjectiles) {
            if (proj.active) {
                drawCircle(proj.x, proj.y, 5.0f, 1.0f, 0.0f, 0.0f);
            }
        }

        displayScore();
    }

    glFlush();
}

// Keyboard function
void keyboard(unsigned char key, int x, int y) {
    if (gameOver || gameWin) return;

    if (key == 'w') shipY += 10.0f;
    if (key == 's') shipY -= 10.0f;
    if (key == 'a') shipX -= 10.0f;
    if (key == 'd') shipX += 10.0f;
    if (key == ' ') {
        playerProjectiles.push_back({ shipX, shipY, true });
    }
    glutPostRedisplay();
}

// Initialize OpenGL
void init() {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    loadTextures();
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB);
    glutInitWindowSize(800, 600);
    glutCreateWindow("2D Space Adventure");
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0.0, 800.0, 0.0, 600.0);
    init();
    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutTimerFunc(0, timer, 0);
    glutMainLoop();
    return 0;
}
