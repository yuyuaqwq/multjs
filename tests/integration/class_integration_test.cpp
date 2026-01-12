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

// ==================== 类字段 ====================

TEST_F(ClassIntegrationTest, InstanceFields) {
    // 测试实例字段
    AssertEq(R"(
        class Point {
            x = 0;
            y = 0;

            constructor(x, y) {
                this.x = x;
                this.y = y;
            }

            getX() {
                return this.x;
            }

            getY() {
                return this.y;
            }
        }

        const p = new Point(10, 20);
        p.getX() + p.getY();
    )", Value(30)); // 10 + 20 = 30
}

TEST_F(ClassIntegrationTest, InstanceFieldsWithInitialization) {
    // 测试带初始值的实例字段
    AssertEq(R"(
        class Counter {
            count = 0;
            step = 1;

            increment() {
                this.count += this.step;
                return this.count;
            }
        }

        const counter = new Counter();
        counter.increment();
        counter.increment();
        counter.count;
    )", Value(2)); // 初始0, 两次increment后变为2
}

TEST_F(ClassIntegrationTest, MultipleInstanceFields) {
    // 测试多个实例字段
    AssertEq(R"(
        class Person {
            name = '';
            age = 0;
            active = true;

            constructor(name, age) {
                this.name = name;
                this.age = age;
            }

            getInfo() {
                return this.name + ':' + this.age;
            }
        }

        const person = new Person('Alice', 30);
        person.getInfo();
    )", Value("Alice:30"));
}

TEST_F(ClassIntegrationTest, StaticFields) {
    // 测试静态字段
    AssertEq(R"(
        class Config {
            static version = '1.0.0';
            static debug = true;
            static maxConnections = 100;

            static getVersion() {
                return Config.version;
            }
        }

        Config.getVersion();
    )", Value("1.0.0"));
}

TEST_F(ClassIntegrationTest, StaticFieldsAccess) {
    // 测试静态字段访问
    AssertEq(R"(
        class Constants {
            static PI = 3.14159;
            static E = 2.71828;
        }

        Constants.PI + Constants.E;
    )", Value(5.85987)); // approximately
}

TEST_F(ClassIntegrationTest, StaticFieldModification) {
    // 测试静态字段修改
    AssertEq(R"(
        class Counter {
            static count = 0;

            static increment() {
                Counter.count += 1;
                return Counter.count;
            }
        }

        Counter.increment();
        Counter.increment();
        Counter.increment();
        Counter.count;
    )", Value(3));
}

TEST_F(ClassIntegrationTest, MixedInstanceAndStaticFields) {
    // 测试混合实例和静态字段
    AssertEq(R"(
        class User {
            static userCount = 0;
            userId = 0;

            constructor(name) {
                this.userId = User.userCount;
                this.name = name;
                User.userCount += 1;
            }

            getId() {
                return this.userId;
            }

            static getTotalUsers() {
                return User.userCount;
            }
        }

        const u1 = new User('Alice');
        const u2 = new User('Bob');
        const u3 = new User('Charlie');
        u1.getId() + u2.getId() + u3.getId() + User.getTotalUsers();
    )", Value(6)); // 0 + 1 + 2 + 3 = 6
}

// ==================== 计算属性名 ====================

TEST_F(ClassIntegrationTest, ComputedPropertyNames) {
    // 测试计算属性名
    AssertEq(R"(
        const methodName = 'getValue';

        class MyClass {
            value = 42;

            [methodName]() {
                return this.value;
            }
        }

        const obj = new MyClass();
        obj.getValue();
    )", Value(42));
}

TEST_F(ClassIntegrationTest, ComputedPropertyNamesWithStrings) {
    // 测试字符串计算属性名
    AssertEq(R"(
        class MyClass {
            ['add'](a, b) {
                return a + b;
            }

            ['multiply'](a, b) {
                return a * b;
            }
        }

        const obj = new MyClass();
        obj.add(5, 3) + obj.multiply(2, 4);
    )", Value(16)); // 8 + 8 = 16
}

TEST_F(ClassIntegrationTest, ComputedPropertyNamesWithExpressions) {
    // 测试表达式计算属性名
    AssertEq(R"(
        const prefix = 'get';
        const suffix = 'Value';

        class MyClass {
            value = 100;

            [prefix + suffix]() {
                return this.value;
            }
        }

        const obj = new MyClass();
        obj.getValue();
    )", Value(100));
}

// ==================== 复杂类定义 ====================

TEST_F(ClassIntegrationTest, ComplexClassWithAllFeatures) {
    // 测试包含所有特性的复杂类
    AssertEq(R"(
        class Rectangle {
            width = 0;
            height = 0;
            static count = 0;

            constructor(width, height) {
                this.width = width;
                this.height = height;
                Rectangle.count += 1;
            }

            get area() {
                return this.width * this.height;
            }

            set dimensions(size) {
                this.width = size;
                this.height = size;
            }

            static getCreatedCount() {
                return Rectangle.count;
            }

            getArea() {
                return this.area;
            }
        }

        const r1 = new Rectangle(5, 10);
        const r2 = new Rectangle(3, 4);
        r1.getArea() + r2.getArea() + Rectangle.getCreatedCount();
    )", Value(64)); // 50 + 12 + 2 = 64
}

TEST_F(ClassIntegrationTest, ClassWithPrivateLikeFields) {
    // 测试使用下划线前缀的"私有"字段约定
    AssertEq(R"(
        class BankAccount {
            _balance = 0;

            constructor(initialBalance) {
                this._balance = initialBalance;
            }

            deposit(amount) {
                this._balance += amount;
                return this._balance;
            }

            withdraw(amount) {
                if (amount <= this._balance) {
                    this._balance -= amount;
                }
                return this._balance;
            }

            getBalance() {
                return this._balance;
            }
        }

        const account = new BankAccount(100);
        account.deposit(50);
        account.withdraw(30);
        account.getBalance();
    )", Value(120)); // 100 + 50 - 30 = 120
}

// ==================== 边界情况 ====================

TEST_F(ClassIntegrationTest, ClassExpressionWithoutName) {
    // 测试匿名类表达式
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

TEST_F(ClassIntegrationTest, ClassExpressionAsFunctionParameter) {
    // 测试类表达式作为函数参数
    AssertEq(R"(
        function createInstance(ClassDef) {
            return new ClassDef(10, 20);
        }

        const Point = class {
            constructor(x, y) {
                this.x = x;
                this.y = y;
            }

            sum() {
                return this.x + this.y;
            }
        };

        const p = createInstance(Point);
        p.sum();
    )", Value(30));
}

TEST_F(ClassIntegrationTest, ClassReturnedFromFunction) {
    // 测试从函数返回类
    AssertEq(R"(
        function createClass() {
            return class {
                constructor(value) {
                    this.value = value;
                }

                getValue() {
                    return this.value;
                }
            };
        }

        const MyClass = createClass();
        const obj = new MyClass(99);
        obj.getValue();
    )", Value(99));
}

TEST_F(ClassIntegrationTest, FieldInitializationOrder) {
    // 测试字段初始化顺序
    AssertTrue(R"(
        class OrderTest {
            a = this.getValue(1);
            b = this.getValue(2);
            c = this.getValue(3);

            getValue(val) {
                return val;
            }

            getSum() {
                return this.a + this.b + this.c;
            }
        }

        const obj = new OrderTest();
        obj.getSum() === 6;
    )");
}

TEST_F(ClassIntegrationTest, FieldOverridesConstructor) {
    // 测试字段是否在构造函数之前初始化
    AssertEq(R"(
        class Test {
            value = 10;

            constructor(newValue) {
                this.value = newValue;
            }
        }

        const obj = new Test(20);
        obj.value;
    )", Value(20)); // 构造函数应该覆盖字段初始值
}

TEST_F(ClassIntegrationTest, StaticFieldAccessViaThis) {
    // 测试通过this访问静态字段（应该在方法中访问）
    AssertEq(R"(
        class Test {
            static value = 100;

            static getValue() {
                return this.value;
            }
        }

        Test.getValue();
    )", Value(100));
}

TEST_F(ClassIntegrationTest, MultipleStaticFields) {
    // 测试多个静态字段
    AssertEq(R"(
        class Config {
            static API_URL = 'https://api.example.com';
            static TIMEOUT = 5000;
            static RETRY_COUNT = 3;

            static getConfig() {
                return Config.TIMEOUT + Config.RETRY_COUNT;
            }
        }

        Config.getConfig();
    )", Value(5003)); // 5000 + 3 = 5003
}

TEST_F(ClassIntegrationTest, ClassFieldsWithComplexValues) {
    // 测试复杂值的字段
    AssertEq(R"(
        class DataHolder {
            data = [1, 2, 3, 4, 5];
            config = { min: 0, max: 100 };
            calc = () => 42;

            getSum() {
                let sum = 0;
                for (let i = 0; i < this.data.length; i += 1) {
                    sum += this.data[i];
                }
                return sum;
            }
        }

        const holder = new DataHolder();
        holder.getSum();
    )", Value(15)); // 1+2+3+4+5 = 15
}

TEST_F(ClassIntegrationTest, EmptyClassFields) {
    // 测试空初始值的字段
    AssertTrue(R"(
        class Test {
            empty;
            nullValue = null;

            isEmpty() {
                return this.empty === undefined;
            }
        }

        const obj = new Test();
        obj.isEmpty();
    )");
}

// ==================== 类字段和方法的交互 ====================

TEST_F(ClassIntegrationTest, FieldsAndMethodsInteraction) {
    // 测试字段和方法的交互
    AssertEq(R"(
        class Calculator {
            result = 0;

            add(value) {
                this.result += value;
                return this;
            }

            multiply(value) {
                this.result *= value;
                return this;
            }

            getResult() {
                return this.result;
            }
        }

        const calc = new Calculator();
        calc.add(10).multiply(5).add(5).getResult();
    )", Value(55)); // ((0 + 10) * 5) + 5 = 55
}

TEST_F(ClassIntegrationTest, StaticFieldsAndInstanceFields) {
    // 测试静态字段和实例字段的交互
    AssertEq(R"(
        class IdGenerator {
            static nextId = 1;
            id = 0;

            constructor() {
                this.id = IdGenerator.nextId;
                IdGenerator.nextId += 1;
            }

            static getNextId() {
                return IdGenerator.nextId;
            }

            getId() {
                return this.id;
            }
        }

        const obj1 = new IdGenerator();
        const obj2 = new IdGenerator();
        const obj3 = new IdGenerator();
        obj1.getId() + obj2.getId() + obj3.getId() + IdGenerator.getNextId();
    )", Value(10)); // 1 + 2 + 3 + 4 = 10
}

// ==================== 错误处理 ====================

TEST_F(ClassIntegrationTest, AccessingUndefinedField) {
    // 测试访问未定义的字段
    AssertTrue(R"(
        class Test {
            defined = 42;

            checkUndefined() {
                return this.undefined === undefined;
            }
        }

        const obj = new Test();
        obj.checkUndefined();
    )");
}

TEST_F(ClassIntegrationTest, ModifyingStaticField) {
    // 测试修改静态字段
    AssertEq(R"(
        class Config {
            static value = 10;
        }

        Config.value = 20;
        Config.value;
    )", Value(20));
}

} // namespace mjs::test
