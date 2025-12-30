// Class basic test
class Person {
    constructor(name: string, age: number) {
        this.name = name;
        this.age = age;
    }

    greet() {
        return `Hello, I'm ${this.name}`;
    }

    get info() {
        return `${this.name} is ${this.age} years old`;
    }

    set info(value: string) {
        // setter implementation
    }
}

// Class with extends
class Student extends Person {
    constructor(name: string, age: number, grade: number) {
        super(name, age);
        this.grade = grade;
    }

    study() {
        return `${this.name} is studying`;
    }

    static createFromName(name: string) {
        return new Student(name, 18, 12);
    }
}

// Class with fields
class Rectangle {
    width: number = 0;
    height: number = 0;

    constructor(width: number, height: number) {
        this.width = width;
        this.height = height;
    }

    get area() {
        return this.width * this.height;
    }

    static createSquare(size: number) {
        return new Rectangle(size, size);
    }
}

// Anonymous class expression
const AnonymousClass = class {
    constructor() {
        this.value = 42;
    }

    getValue() {
        return this.value;
    }
};

// Named class expression
const NamedClassExpr = class NamedClass {
    constructor() {
        this.name = "NamedClass";
    }
};
