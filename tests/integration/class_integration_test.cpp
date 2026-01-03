/**
 * @file class_integration_test.cpp
 * @brief 类与继承集成测试
 *
 * @copyright Copyright (c) 2025 yuyuaqwq
 * @license MIT License
 */

#include "test_helper.h"
#include <gtest/gtest.h>

namespace mjs::test {

/**
 * @class ClassIntegrationTest
 * @brief 类与继承集成测试
 */
class ClassIntegrationTest : public IntegrationTestHelper {
};

// ==================== 类基础 ====================

TEST_F(ClassIntegrationTest, SimpleClass) {
    // 测试简单的类
    AssertEq(R"(
        class Point {
            constructor(x, y) {
                this.x = x;
                this.y = y;
            }

            getDistance() {
                return this.x * this.x + this.y * this.y;
            }
        }

        const p = new Point(3, 4);
        p.getDistance();
    )", Value(25)); // 3² + 4² = 25
}

TEST_F(ClassIntegrationTest, ClassWithGetterSetter) {
    // 测试带getter和setter的类
    AssertEq(R"(
        class Rectangle {
            constructor(width, height) {
                this._width = width;
                this._height = height;
            }

            get area() {
                return this._width * this._height;
            }

            set width(value) {
                this._width = value;
            }

            get width() {
                return this._width;
            }
        }

        const rect = new Rectangle(5, 10);
        rect.area;
    )", Value(50)); // 5 * 10 = 50
}

TEST_F(ClassIntegrationTest, StaticMethods) {
    // 测试静态方法
    AssertEq(R"(
        class MathUtil {
            static add(a, b) {
                return a + b;
            }

            static multiply(a, b) {
                return a * b;
            }
        }

        MathUtil.add(5, 3) + MathUtil.multiply(2, 4);
    )", Value(16)); // 8 + 8 = 16
}

TEST_F(ClassIntegrationTest, ClassWithDefaultConstructor) {
    // 测试默认构造函数
    AssertTrue(R"(
        class SimpleClass {
            method() {
                return 42;
            }
        }

        const obj = new SimpleClass();
        obj.method() === 42;
    )");
}

// ==================== 继承 ====================

TEST_F(ClassIntegrationTest, SimpleInheritance) {
    // 测试简单的继承
    AssertEq(R"(
        class Animal {
            constructor(name) {
                this.name = name;
            }

            speak() {
                return this.name + ' makes a sound';
            }
        }

        class Dog extends Animal {
            constructor(name, breed) {
                super(name);
                this.breed = breed;
            }

            speak() {
                return this.name + ' barks';
            }

            getBreed() {
                return this.breed;
            }
        }

        const dog = new Dog('Rex', 'German Shepherd');
        dog.speak();
    )", Value("Rex barks"));
}

TEST_F(ClassIntegrationTest, SuperCall) {
    // 测试super调用
    AssertEq(R"(
        class Parent {
            constructor(x) {
                this.x = x;
            }

            getValue() {
                return this.x * 2;
            }
        }

        class Child extends Parent {
            constructor(x, y) {
                super(x);
                this.y = y;
            }

            getValue() {
                return super.getValue() + this.y;
            }
        }

        const child = new Child(5, 10);
        child.getValue();
    )", Value(20)); // 5*2 + 10 = 20
}

TEST_F(ClassIntegrationTest, MethodOverriding) {
    // 测试方法重写
    AssertEq(R"(
        class Base {
            greet() {
                return 'Hello from Base';
            }
        }

        class Derived extends Base {
            greet() {
                return 'Hello from Derived';
            }

            callBaseGreet() {
                return super.greet();
            }
        }

        const obj = new Derived();
        obj.greet() + ' | ' + obj.callBaseGreet();
    )", Value("Hello from Derived | Hello from Base"));
}

TEST_F(ClassIntegrationTest, MultiLevelInheritance) {
    // 测试多层继承
    AssertEq(R"(
        class A {
            method() {
                return 10;
            }
        }

        class B extends A {
            method() {
                return super.method() + 20;
            }
        }

        class C extends B {
            method() {
                return super.method() + 30;
            }
        }

        const obj = new C();
        obj.method();
    )", Value(60)); // 10 + 20 + 30 = 60
}

// ==================== 原型链 ====================

TEST_F(ClassIntegrationTest, PrototypeChainLookup) {
    // 测试原型链查找
    AssertEq(R"(
        class Base {
            getValue() {
                return 100;
            }
        }

        class Derived extends Base {
        }

        const obj = new Derived();
        obj.getValue();
    )", Value(100));
}

TEST_F(ClassIntegrationTest, PrototypeProperty) {
    // 测试原型属性
    AssertTrue(R"(
        class MyClass {
            constructor() {
                this.instanceProperty = 'instance';
            }
        }

        MyClass.prototype.prototypeProperty = 'prototype';

        const obj = new MyClass();
        obj.instanceProperty === 'instance' && obj.prototypeProperty === 'prototype';
    )");
}

// ==================== 复杂场景 ====================

TEST_F(ClassIntegrationTest, Polymorphism) {
    // 测试多态
    AssertEq(R"(
        class Shape {
            area() {
                return 0;
            }
        }

        class Rectangle extends Shape {
            constructor(width, height) {
                super();
                this.width = width;
                this.height = height;
            }

            area() {
                return this.width * this.height;
            }
        }

        class Circle extends Shape {
            constructor(radius) {
                super();
                this.radius = radius;
            }

            area() {
                return 3.14 * this.radius * this.radius;
            }
        }

        const shapes = [
            new Rectangle(5, 10),
            new Circle(5),
            new Rectangle(2, 3)
        ];

        let totalArea = 0;
        for (let i = 0; i < shapes.length; i += 1) {
            totalArea += shapes[i].area();
        }
        totalArea;
    )", Value(128.5)); // 50 + 78.5 + 6 = 134.5 (approximately)
}

TEST_F(ClassIntegrationTest, CompositionOverInheritance) {
    // 测试组合优于继承
    AssertEq(R"(
        class Logger {
            log(message) {
                return 'LOG: ' + message;
            }
        }

        class DataProcessor {
            constructor(logger) {
                this.logger = logger;
            }

            process(data) {
                return this.logger.log('Processing: ' + data);
            }
        }

        const logger = new Logger();
        const processor = new DataProcessor(logger);
        processor.process('test data');
    )", Value("LOG: Processing: test data"));
}

TEST_F(ClassIntegrationTest, FactoryPattern) {
    // 测试工厂模式
    AssertEq(R"(
        class Car {
            constructor(brand) {
                this.brand = brand;
            }

            drive() {
                return this.brand + ' is driving';
            }
        }

        class CarFactory {
            static createCar(brand) {
                return new Car(brand);
            }
        }

        const car1 = CarFactory.createCar('Toyota');
        const car2 = CarFactory.createCar('Honda');
        car1.drive() + ' | ' + car2.drive();
    )", Value("Toyota is driving | Honda is driving"));
}

TEST_F(ClassIntegrationTest, ObserverPattern) {
    // 测试观察者模式
    AssertEq(R"(
        class Subject {
            constructor() {
                this.observers = [];
            }

            subscribe(observer) {
                this.observers.push(observer);
            }

            notify(data) {
                let result = 0;
                for (let i = 0; i < this.observers.length; i += 1) {
                    result += this.observers[i](data);
                }
                return result;
            }
        }

        const subject = new Subject();

        subject.subscribe(function(x) {
            return x * 2;
        });

        subject.subscribe(function(x) {
            return x * 3;
        });

        subject.notify(5);
    )", Value(25)); // 5*2 + 5*3 = 25
}

TEST_F(ClassIntegrationTest, SingletonPattern) {
    // 测试单例模式
    AssertEq(R"(
        class Singleton {
            constructor() {
                if (Singleton.instance) {
                    return Singleton.instance;
                }
                this.value = 0;
                Singleton.instance = this;
            }

            increment() {
                this.value += 1;
                return this.value;
            }
        }

        const s1 = new Singleton();
        const s2 = new Singleton();

        s1.increment();
        s1.increment();
        s2.increment();
        s1.value;
    )", Value(3)); // s1和s2是同一个实例
}

TEST_F(ClassIntegrationTest, MixinPattern) {
    // 测试Mixin模式
    AssertEq(R"(
        const Serializable = {
            serialize() {
                return JSON.stringify(this);
            }
        };

        class User {
            constructor(name, email) {
                this.name = name;
                this.email = email;
            }
        }

        Object.assign(User.prototype, Serializable);

        const user = new User('Alice', 'alice@example.com');
        const serialized = user.serialize();
        serialized.includes('Alice');
    )", Value(true));
}

// ==================== 边界情况 ====================

TEST_F(ClassIntegrationTest, ClassExpression) {
    // 测试类表达式
    AssertEq(R"(
        const MyClass = class {
            constructor(value) {
                this.value = value;
            }

            getValue() {
                return this.value;
            }
        };

        const obj = new MyClass(42);
        obj.getValue();
    )", Value(42));
}

TEST_F(ClassIntegrationTest, ClassWithoutConstructor) {
    // 测试没有构造函数的类
    AssertEq(R"(
        class SimpleClass {
            method() {
                return 100;
            }
        }

        const obj = new SimpleClass();
        obj.method();
    )", Value(100));
}

TEST_F(ClassIntegrationTest, ExtendingBuiltInClasses) {
    // 测试扩展内置类
    AssertEq(R"(
        class ExtendedArray extends Array {
            get first() {
                return this.length > 0 ? this[0] : undefined;
            }

            get last() {
                return this.length > 0 ? this[this.length - 1] : undefined;
            }
        }

        const arr = new ExtendedArray(1, 2, 3, 4, 5);
        arr.first + arr.last;
    )", Value(6)); // 1 + 5 = 6
}

} // namespace mjs::test
