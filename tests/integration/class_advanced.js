// Class advanced features test - 测试class的高级特性

// 测试1: 带有getter和setter的类
class Temperature {
    constructor(celsius) {
        this._celsius = celsius;
    }

    get celsius() {
        return this._celsius;
    }

    set celsius(value) {
        this._celsius = value;
    }

    get fahrenheit() {
        return this._celsius * 9 / 5 + 32;
    }

    set fahrenheit(value) {
        this._celsius = (value - 32) * 5 / 9;
    }
}

const temp = new Temperature(25);
console.log(`Celsius: ${temp.celsius}`);
console.log(`Fahrenheit: ${temp.fahrenheit}`);

temp.fahrenheit = 86;
console.log(`After setting to 86°F, Celsius: ${temp.celsius}`);

// 测试2: 计算属性名
const methodName = "greet";
class ComputedProperties {
    constructor(name) {
        this.name = name;
    }

    [methodName]() {
        return `Hello, ${this.name}`;
    }

    get ["info"]() {
        return `Name: ${this.name}`;
    }
}

const computed = new ComputedProperties("World");
console.log(computed.greet());
console.log(computed.info);

// 测试3: 静态getter/setter
class Config {
    static _version = "1.0.0";

    static get version() {
        return Config._version;
    }

    static set version(value) {
        Config._version = value;
    }
}

console.log(`Config version: ${Config.version}`);
Config.version = "2.0.0";
console.log(`Updated version: ${Config.version}`);

// 测试4: 混合字段和方法
class Counter {
    count = 0;
    static instances = 0;

    constructor(initial = 0) {
        this.count = initial;
        Counter.instances++;
    }

    increment() {
        this.count++;
        return this;
    }

    decrement() {
        this.count--;
        return this;
    }

    getValue() {
        return this.count;
    }

    static getTotalInstances() {
        return Counter.instances;
    }
}

const counter1 = new Counter(5);
const counter2 = new Counter(10);
const counter3 = new Counter();

console.log(`Counter1: ${counter1.getValue()}`);
console.log(`Counter2: ${counter2.getValue()}`);
console.log(`Counter3: ${counter3.getValue()}`);
console.log(`Total instances: ${Counter.getTotalInstances()}`);

// 测试5: 方法链
console.log(counter1.increment().increment().decrement().getValue());

// 测试6: 类作为一等公民
function createClass(className) {
    return class {
        constructor() {
            this.name = className;
        }

        getName() {
            return this.name;
        }
    };
}

const DynamicClass = createClass("DynamicClass");
const dynamic = new DynamicClass();
console.log(dynamic.getName());

// 测试7: 在数组中存储类
const classes = [
    class {
        constructor(value) {
            this.value = value;
        }
    },
    class {
        constructor(value) {
            this.value = value * 2;
        }
    }
];

const instance1 = new classes[0](5);
const instance2 = new classes[1](5);
console.log(`Instance1: ${instance1.value}`);
console.log(`Instance2: ${instance2.value}`);

// 测试8: 多级继承
class Vehicle {
    constructor(speed) {
        this.speed = speed;
    }

    move() {
        return `Moving at ${this.speed} km/h`;
    }
}

class Car extends Vehicle {
    constructor(speed, brand) {
        super(speed);
        this.brand = brand;
    }

    move() {
        return `${this.brand} car is ${super.move()}`;
    }
}

class ElectricCar extends Car {
    constructor(speed, brand, battery) {
        super(speed, brand);
        this.battery = battery;
    }

    getBatteryInfo() {
        return `Battery: ${this.battery}%`;
    }
}

const tesla = new ElectricCar(120, "Tesla", 85);
console.log(tesla.move());
console.log(tesla.getBatteryInfo());

// 测试9: 类方法返回this
class Builder {
    constructor() {
        this.value = 0;
    }

    add(x) {
        this.value += x;
        return this;
    }

    subtract(x) {
        this.value -= x;
        return this;
    }

    multiply(x) {
        this.value *= x;
        return this;
    }

    getResult() {
        return this.value;
    }
}

const result = new Builder()
    .add(10)
    .multiply(2)
    .subtract(5)
    .getResult();
console.log(`Builder result: ${result}`);

// 测试10: 类的toString和valueOf
class Money {
    constructor(amount, currency) {
        this.amount = amount;
        this.currency = currency;
    }

    toString() {
        return `${this.amount} ${this.currency}`;
    }

    valueOf() {
        return this.amount;
    }
}

const money = new Money(100, "USD");
console.log(money.toString());
console.log(`Value: ${money.valueOf()}`);
