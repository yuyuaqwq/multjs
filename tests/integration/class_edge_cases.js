// Class edge cases test - 测试class的边缘情况

// 测试1: 空类
class EmptyClass {
}

const empty = new EmptyClass();
console.log("Empty class instance:", typeof empty);

// 测试2: 只有静态成员的类
class StaticOnly {
    static value = 42;

    static getValue() {
        return StaticOnly.value;
    }
}

console.log("Static only:", StaticOnly.getValue());

// 测试3: 只有字段的类
class FieldsOnly {
    x = 1;
    y = 2;
    z = 3;
}

const fields = new FieldsOnly();
console.log("Fields only:", fields.x, fields.y, fields.z);

// 测试4: 多个getter/setter
class MultipleGettersSetters {
    constructor() {
        this._x = 0;
        this._y = 0;
    }

    get x() { return this._x; }
    set x(value) { this._x = value; }

    get y() { return this._y; }
    set y(value) { this._y = value; }

    get sum() { return this._x + this._y; }
}

const coords = new MultipleGettersSetters();
coords.x = 10;
coords.y = 20;
console.log("Multiple getters/setters:", coords.sum);

// 测试5: 类方法的默认值
class DefaultValues {
    constructor(a = 1, b = 2, c = 3) {
        this.a = a;
        this.b = b;
        this.c = c;
    }
}

const defaults1 = new DefaultValues();
const defaults2 = new DefaultValues(10, 20);
console.log("Default values 1:", defaults1.a, defaults1.b, defaults1.c);
console.log("Default values 2:", defaults2.a, defaults2.b, defaults2.c);

// 测试6: 静态初始化顺序
class InitOrder {
    static step1 = "1";
    static step2 = InitOrder.step1 + "2";
    static step3 = InitOrder.step2 + "3";
}

console.log("Init order:", InitOrder.step1, InitOrder.step2, InitOrder.step3);

// 测试7: 多层继承链
class A {
    constructor() {
        this.a = "A";
    }

    methodA() {
        return "methodA";
    }
}

class B extends A {
    constructor() {
        super();
        this.b = "B";
    }

    methodB() {
        return "methodB";
    }
}

class C extends B {
    constructor() {
        super();
        this.c = "C";
    }

    methodC() {
        return "methodC";
    }
}

const c = new C();
console.log("Multi-level inheritance:", c.a, c.b, c.c);
console.log("Multi-level methods:", c.methodA(), c.methodB(), c.methodC());

// 测试8: 类与函数混合使用
function factory() {
    return class {
        constructor(value) {
            this.value = value;
        }

        getValue() {
            return this.value;
        }
    };
}

const FactoryClass = factory();
const factoryInstance = new FactoryClass(100);
console.log("Factory pattern:", factoryInstance.getValue());

// 测试9: 类作为参数
function createInstance(ClassRef, ...args) {
    return new ClassRef(...args);
}

class Point {
    constructor(x, y) {
        this.x = x;
        this.y = y;
    }
}

const point = createInstance(Point, 5, 10);
console.log("Class as parameter:", point.x, point.y);

// 测试10: 动态方法名
const dynamicName = "dynamic";
class DynamicMethod {
    constructor() {
        this.value = 42;
    }

    [dynamicName]() {
        return this.value;
    }
}

const dm = new DynamicMethod();
console.log("Dynamic method name:", dm.dynamic());

// 测试11: 返回this的方法链
class Chain {
    constructor() {
        this.value = 0;
    }

    add(v) {
        this.value += v;
        return this;
    }

    subtract(v) {
        this.value -= v;
        return this;
    }

    multiply(v) {
        this.value *= v;
        return this;
    }

    getResult() {
        return this.value;
    }
}

const chainResult = new Chain().add(10).multiply(2).subtract(5).getResult();
console.log("Method chaining:", chainResult);

// 测试12: 静态和实例同名成员
class SameName {
    static value = "static";

    constructor() {
        this.value = "instance";
    }

    static getValue() {
        return SameName.value;
    }

    getValue() {
        return this.value;
    }
}

const sameName = new SameName();
console.log("Same name static:", SameName.getValue());
console.log("Same name instance:", sameName.getValue());

// 测试13: 类字段的复杂初始化
class ComplexFields {
    x = 1 + 2;
    y = this.x + 3;
    z = [1, 2, 3];
    obj = { a: 1, b: 2 };
}

const complex = new ComplexFields();
console.log("Complex fields:", complex.x, complex.y, complex.z, complex.obj);

// 测试14: 静态字段的复杂初始化
class ComplexStatic {
    static x = 1 + 2;
    static y = ComplexStatic.x + 3;
    static obj = { sum: ComplexStatic.x + ComplexStatic.y };
}

console.log("Complex static:", ComplexStatic.x, ComplexStatic.y, ComplexStatic.obj);

// 测试15: 类表达式立即实例化
const instance = new class {
    constructor() {
        this.value = 100;
    }

    getValue() {
        return this.value;
    }
}();

console.log("IIFE class:", instance.getValue());

// 测试16: 空构造函数
class EmptyConstructor {
    constructor() {
    }

    method() {
        return "method";
    }
}

const emptyCtor = new EmptyConstructor();
console.log("Empty constructor:", emptyCtor.method());

// 测试17: 只有构造函数
class OnlyConstructor {
    constructor(x) {
        this.x = x;
    }
}

const onlyCtor = new OnlyConstructor(42);
console.log("Only constructor:", onlyCtor.x);

// 测试18: 静态getter/setter
class StaticGetterSetter {
    static _value = 0;

    static get value() {
        return StaticGetterSetter._value;
    }

    static set value(v) {
        StaticGetterSetter._value = v;
    }
}

StaticGetterSetter.value = 100;
console.log("Static getter/setter:", StaticGetterSetter.value);

// 测试19: 混合静态和实例getter/setter
class MixedGettersSetters {
    static _staticValue = 1;
    _instanceValue = 2;

    static get staticProp() {
        return MixedGettersSetters._staticValue;
    }

    static set staticProp(v) {
        MixedGettersSetters._staticValue = v;
    }

    get instanceProp() {
        return this._instanceValue;
    }

    set instanceProp(v) {
        this._instanceValue = v;
    }
}

MixedGettersSetters.staticProp = 10;
const mixed = new MixedGettersSetters();
mixed.instanceProp = 20;
console.log("Mixed getter/setter:", MixedGettersSetters.staticProp, mixed.instanceProp);

// 测试20: 类继承中的super
class Parent {
    constructor(x) {
        this.x = x;
    }

    method() {
        return "parent";
    }
}

class Child extends Parent {
    constructor(x, y) {
        super(x);
        this.y = y;
    }

    method() {
        return super.method() + " -> child";
    }
}

const child = new Child(1, 2);
console.log("Super call:", child.method());
