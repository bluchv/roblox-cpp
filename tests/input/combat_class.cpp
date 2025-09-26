#include <string>

class Combat {
private:
  int health;
  int damage;
  std::string weapon;

public:
  Combat(int initialHealth, int weaponDamage) {
    health = initialHealth;
    damage = weaponDamage;
    weapon = "sword";
  }

  void attack(Combat &target) { target.takeDamage(damage); }

  void takeDamage(int amount) {
    health = health - amount;
    if (health < 0) {
      health = 0;
    }
  }

  int getHealth() { return health; }

  bool isAlive() { return health > 0; }

  void heal(int amount) {
    health = health + amount;
    if (health > 100) {
      health = 100;
    }
  }
};

int main() {
  Combat player(100, 25);
  Combat enemy(80, 15);

  player.attack(enemy);
  enemy.attack(player);

  if (player.isAlive() && enemy.isAlive()) {
    player.heal(10);
  }

  return 0;
}
