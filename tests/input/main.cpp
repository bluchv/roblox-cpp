int playerHealth = 100;
int playerMana = 50;
double playerX = 10.5;

void heal(int amount) { playerHealth = playerHealth + amount; }

void setPosition(double x, double y) {
  playerX = x;
  playerMana = 25;
}

int getHealth() { return playerHealth; }

int main() {
  int health = getHealth();
  return 0;
}
