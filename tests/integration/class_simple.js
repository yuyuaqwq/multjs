// 简单的class测试

// 测试1: 基本的类声明
class Animal {
    constructor(name) {
        this.name = name;
    }

    speak() {
        return this.name + " makes a sound";
    }
}

// 测试2: 类继承
class Dog extends Animal {
    constructor(name, breed) {
        super(name);
        this.breed = breed;
    }

    speak() {
        return this.name + " barks";
    }

    static getSpecies() {
        return "Canis familiaris";
    }
}

// 测试3: 类字段
class Rectangle {
    width = 0;
    height = 0;

    constructor(w, h) {
        this.width = w;
        this.height = h;
    }

    get area() {
        return this.width * this.height;
    }
}

// 测试4: 匿名类表达式
const myClass = class {
    constructor(value) {
        this.value = value;
    }

    getValue() {
        return this.value;
    }
};

// 测试5: 静态方法和字段
class MathHelper {
    static PI = 3.14159;

    static circleArea(radius) {
        return MathHelper.PI * radius * radius;
    }
}
