// Class execution test - 测试class的运行时行为

// 测试1: 基本类实例化
class Person {
    constructor(name, age) {
        this.name = name;
        this.age = age;
    }

    greet() {
        return `Hello, I'm ${this.name}`;
    }

    get info() {
        return `${this.name} is ${this.age} years old`;
    }
}

const person1 = new Person("Alice", 25);
console.log(person1.greet());
console.log(person1.info);

// 测试2: 类继承
class Student extends Person {
    constructor(name, age, grade) {
        super(name, age);
        this.grade = grade;
    }

    study() {
        return `${this.name} is studying in grade ${this.grade}`;
    }

    static createFromName(name) {
        return new Student(name, 18, 12);
    }
}

const student1 = new Student("Bob", 16, 10);
console.log(student1.greet());
console.log(student1.study());

// 测试3: 静态方法
const student2 = Student.createFromName("Charlie");
console.log(student2.name);

// 测试4: 类字段
class Rectangle {
    width = 0;
    height = 0;

    constructor(width, height) {
        this.width = width;
        this.height = height;
    }

    get area() {
        return this.width * this.height;
    }

    static createSquare(size) {
        return new Rectangle(size, size);
    }
}

const rect1 = new Rectangle(5, 3);
console.log(`Rectangle area: ${rect1.area}`);

const square = Rectangle.createSquare(4);
console.log(`Square area: ${square.area}`);

// 测试5: 匿名类表达式
const AnonymousClass = class {
    constructor(value) {
        this.value = value;
    }

    getValue() {
        return this.value;
    }
};

const anon = new AnonymousClass(42);
console.log(`Anonymous class value: ${anon.getValue()}`);

// 测试6: 命名类表达式
const NamedClassExpr = class NamedClass {
    constructor() {
        this.name = "NamedClass";
    }

    getName() {
        return this.name;
    }
};

const named = new NamedClassExpr();
console.log(named.getName());

// 测试7: 静态字段和方法
class MathHelper {
    static PI = 3.14159;

    static circleArea(radius) {
        return MathHelper.PI * radius * radius;
    }
}

console.log(`PI: ${MathHelper.PI}`);
console.log(`Circle area with radius 5: ${MathHelper.circleArea(5)}`);

// 测试8: 只有静态成员的类
class Utility {
    static log(msg) {
        console.log(`[LOG] ${msg}`);
    }

    static warn(msg) {
        console.log(`[WARN] ${msg}`);
    }
}

Utility.log("Test message");
Utility.warn("Warning message");

// 测试9: 链式继承
class Animal {
    constructor(name) {
        this.name = name;
    }

    speak() {
        return `${this.name} makes a sound`;
    }
}

class Dog extends Animal {
    constructor(name, breed) {
        super(name);
        this.breed = breed;
    }

    speak() {
        return `${this.name} barks`;
    }

    static getSpecies() {
        return "Canis familiaris";
    }
}

const dog = new Dog("Buddy", "Golden Retriever");
console.log(dog.speak());
console.log(`Dog species: ${Dog.getSpecies()}`);

// 测试10: 空类
class EmptyClass {
}

const empty = new EmptyClass();
console.log(typeof empty);
