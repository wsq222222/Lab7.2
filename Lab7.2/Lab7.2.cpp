#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>
#include <fstream>
#include <mutex>

std::mutex combatMutex;

// Класс Monster
class Monster {
private:
    std::string name;
    int health;
    int attack;
    int level;

public:
    Monster(const std::string& n, int h, int a, int l)
        : name(n), health(h), attack(a), level(l) {
        std::cout << "Monster " << name << " spawned!\n";
    }

    ~Monster() {
        std::cout << "Monster " << name << " destroyed!\n";
    }

    void displayInfo() const {
        std::cout << "Monster: " << name << "\nHP: " << health << "\nLevel: " << level << std::endl;
    }

    std::string GetName() const { return name; }
    int GetHealth() const { return health; }
    int GetAttack() const { return attack; }
    int GetLevel() const { return level; }

    void TakeDamage(int damage) {
        health -= damage;
        if (health < 0) health = 0;
    }

    bool IsAlive() const { return health > 0; }
};


class Character {
private:
    std::string name;
    int health;
    int attack;
    int level;

public:
    Character(const std::string& n, int h, int a, int l)
        : name(n), health(h), attack(a), level(l) {
        std::cout << "Character " << name << " created!\n";
    }

    ~Character() {
        std::cout << "Character " << name << " destroyed!\n";
    }

    void displayInfo() const {
        std::cout << "Name: " << name << "\nHP: " << health << "\nLevel: " << level << std::endl;
    }

    std::string GetName() const { return name; }
    int GetHealth() const { return health; }
    int GetAttack() const { return attack; }
    int GetLevel() const { return level; }

    void TakeDamage(int damage) {
        health -= damage;
        if (health < 0) health = 0;
    }

    bool IsAlive() const { return health > 0; }
};


template <typename T>
class GameManager {
private:
    std::vector<T*> entities;

public:
    void addEntity(T* entity) {
        entities.push_back(entity);
    }

    void displayAll() const {
        for (const auto* entity : entities) {
            entity->displayInfo();
        }
    }

    void saveToFile(const std::string& filename) const {
        std::ofstream file(filename);
        if (!file) {
            throw std::runtime_error("Failed to open file for writing.");
        }
        for (const auto* entity : entities) {
            file << entity->GetName() << "\n"
                << entity->GetLevel() << "\n"
                << entity->GetHealth() << "\n";
        }
    }

    void loadFromFile(const std::string& filename) {
        std::ifstream file(filename);
        if (!file) {
            throw std::runtime_error("Failed to open file for reading.");
        }

        std::string name;
        int health, level;
        while (file >> name >> level >> health) {
            entities.push_back(new T(name, health, 10, level));
        }
    }

    ~GameManager() {
        for (auto* entity : entities) {
            delete entity;
        }
    }

    std::vector<T*>& getEntities() { return entities; }
};


void combat(Character* hero, Monster* monster) {
    while (hero->IsAlive() && monster->IsAlive()) {
        {
            std::lock_guard<std::mutex> lock(combatMutex);

            monster->TakeDamage(hero->GetAttack());
            std::cout << hero->GetName() << " deals " << hero->GetAttack() << " damage to " << monster->GetName()
                << ". Monster HP: " << monster->GetHealth() << std::endl;

            if (monster->IsAlive()) {
                hero->TakeDamage(monster->GetAttack());
                std::cout << monster->GetName() << " deals " << monster->GetAttack() << " damage to " << hero->GetName()
                    << ". Hero HP: " << hero->GetHealth() << std::endl;
            }
        }
    }

    std::lock_guard<std::mutex> lock(combatMutex);
    if (hero->IsAlive()) {
        std::cout << hero->GetName() << " wins the battle!" << std::endl;
    }
    else {
        std::cout << monster->GetName() << " wins the battle!" << std::endl;
    }
}

int main() {
    try {
        GameManager<Character> characterManager;
        GameManager<Monster> monsterManager;

        Character* hero = new Character("Hero", 100, 20, 10);
        Monster* monster = new Monster("Goblin", 50, 15, 5);

        characterManager.addEntity(hero);
        monsterManager.addEntity(monster);

        std::cout << "Initial state:\n";
        characterManager.displayAll();
        monsterManager.displayAll();

        combat(hero, monster);

        std::cout << "\nFinal state:\n";
        characterManager.displayAll();
        monsterManager.displayAll();

        characterManager.saveToFile("characters.txt");
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}