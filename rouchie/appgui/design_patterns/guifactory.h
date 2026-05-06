#pragma once

#include "../guibase.h"

class Animal {
public:
    using Ptr = std::shared_ptr<Animal>;

public:
    virtual ~Animal() = default;
    virtual std::string eat() = 0;
};

class Dog : public Animal {
public:
    std::string eat() override {
        return "Dog eat meat";
    }
};

class Cat : public Animal {
public:
    std::string eat() override {
        return "Cat eat fish";
    }
};

// 这种方法，每次新建一个动物之后，这个函数必须修改一下
inline Animal::Ptr CreateAnimal(const std::string &animal) {
    if (animal == "Dog") {
        return std::make_shared<Dog>();
    } else if (animal == "Cat") {
        return std::make_shared<Cat>();
    }
    return nullptr;
}

// 这种类型，每次新建一个动物，这个类不需要修改，只需要新建一个动物类和动物工厂类
class AnimalFactory {
public:
    using Ptr = std::shared_ptr<AnimalFactory>;

public:
    virtual ~AnimalFactory() = default;
    std::string eat() { return CreateAnimal()->eat(); }

protected:
    virtual Animal::Ptr CreateAnimal() = 0;
};

class DogFactory : public AnimalFactory {
protected:
    Animal::Ptr CreateAnimal() override { return std::make_shared<Dog>(); }
};

class CatFactory : public AnimalFactory {
protected:
    Animal::Ptr CreateAnimal() override { return std::make_shared<Cat>(); }
};

class GuiFactory : public GuiBase {
public:
    GuiFactory();
    void operator()() override;

private:
    int _nSelected = 0;   // 0: 选项A, 1: 选项B, 2: 选项C
    std::string eat;

    Animal::Ptr _animal;
    AnimalFactory::Ptr _animalFactory;
};

inline GuiFactory::GuiFactory() {
    _animal = CreateAnimal("Dog");
    eat = _animal->eat();
    _animalFactory = std::make_shared<DogFactory>();
}

inline void GuiFactory::operator()() {
    ImGui::Begin("工厂方法");

    const int nSelected = _nSelected;

    ImGui::RadioButton("狗0", &_nSelected, 0);
    ImGui::SameLine();
    ImGui::RadioButton("猫0", &_nSelected, 1);
    ImGui::SameLine();
    ImGui::RadioButton("狗1", &_nSelected, 2);
    ImGui::SameLine();
    ImGui::RadioButton("猫1", &_nSelected, 3);

    if (nSelected != _nSelected && _nSelected == 0) {
        _animal = CreateAnimal("Dog");
        eat = _animal->eat();
    } else if (nSelected != _nSelected && _nSelected == 1) {
        _animal = CreateAnimal("Cat");
        eat = _animal->eat();
    } else if (nSelected != _nSelected && _nSelected == 2) {
        _animalFactory = std::make_shared<DogFactory>();
        eat = _animalFactory->eat();
    } else if (nSelected != _nSelected && _nSelected == 3) {
        _animalFactory = std::make_shared<CatFactory>();
        eat = _animalFactory->eat();
    }

    ImGui::Text("%s", eat.c_str());

    ImGui::End();
}
