/**
 * @file class_advanced_integration_test.cpp
 * @brief 类高级特性集成测试
 *
 * @copyright Copyright (c) 2025
 * @license MIT License
 */

#include "test_helper.h"
#include <gtest/gtest.h>

namespace mjs::test {

/**
 * @class ClassAdvancedIntegrationTest
 * @brief 类高级特性集成测试
 */
class ClassAdvancedIntegrationTest : public IntegrationTestHelper {
};

// ==================== 静态Getter/Setter ====================

TEST_F(ClassAdvancedIntegrationTest, StaticGetterAndSetter) {
    // 测试静态getter和setter
    AssertEq(R"(
        class Config {
            static _version = '1.0.0';

            static get version() {
                return Config._version;
            }

            static set version(v) {
                Config._version = v;
            }
        }

        Config.version;
    )", Value("1.0.0"));
}

TEST_F(ClassAdvancedIntegrationTest, StaticSetterModification) {
    // 测试静态setter修改值
    AssertEq(R"(
        class Counter {
            static _count = 0;

            static get count() {
                return Counter._count;
            }

            static set count(value) {
                Counter._count = value;
            }
        }

        Counter.count = 5;
        Counter.count = 10;
        Counter.count;
    )", Value(10));
}

TEST_F(ClassAdvancedIntegrationTest, StaticGetterSetterWithValidation) {
    // 测试静态getter/setter带验证逻辑
    AssertEq(R"(
        class Temperature {
            static _celsius = 0;

            static get celsius() {
                return Temperature._celsius;
            }

            static set celsius(value) {
                if (value < -273.15) {
                    value = -273.15;
                }
                Temperature._celsius = value;
            }

            static get fahrenheit() {
                return Temperature._celsius * 9 / 5 + 32;
            }
        }

        Temperature.celsius = 25;
        const f1 = Temperature.fahrenheit;

        Temperature.celsius = -300;
        const c = Temperature.celsius;

        const f2 = Temperature.fahrenheit;

        c;
    )", Value(-273.15));
}

TEST_F(ClassAdvancedIntegrationTest, MultipleStaticGetterSetter) {
    // 测试多个静态getter/setter
    AssertEq(R"(
        class Settings {
            static _theme = 'light';
            static _language = 'en';

            static get theme() {
                return Settings._theme;
            }

            static set theme(value) {
                Settings._theme = value;
            }

            static get language() {
                return Settings._language;
            }

            static set language(value) {
                Settings._language = value;
            }
        }

        Settings.theme = 'dark';
        Settings.language = 'zh';

        Settings.theme + ':' + Settings.language;
    )", Value("dark:zh"));
}

// ==================== 实例Getter/Setter高级用法 ====================

TEST_F(ClassAdvancedIntegrationTest, GetterSetterWithComputedProperties) {
    // 测试getter/setter与计算属性结合
    AssertEq(R"(
        const propName = 'data';

        class Container {
            _data = [1, 2, 3];

            get [propName]() {
                return this._data;
            }

            set [propName](value) {
                this._data = value;
            }

            get length() {
                return this._data.length;
            }
        }

        const c = new Container();
        c.data = [4, 5, 6, 7];
        c.length;
    )", Value(4));
}

TEST_F(ClassAdvancedIntegrationTest, GetterSetterChaining) {
    // 测试getter/setter链式调用
    AssertEq(R"(
        class Rectangle {
            _width = 0;
            _height = 0;

            constructor(width, height) {
                this.width = width;
                this.height = height;
            }

            get width() {
                return this._width;
            }

            set width(value) {
                this._width = value < 0 ? 0 : value;
            }

            get height() {
                return this._height;
            }

            set height(value) {
                this._height = value < 0 ? 0 : value;
            }

            get area() {
                return this._width * this._height;
            }

            get perimeter() {
                return 2 * (this._width + this._height);
            }
        }

        const rect = new Rectangle(-5, 10);
        rect.area + rect.perimeter;
    )", Value(20)); // 0*10 + 2*(0+10) = 20
}

// ==================== Async方法 ====================

TEST_F(ClassAdvancedIntegrationTest, AsyncMethodBasic) {
    // 测试基本的async方法
    AssertEq(R"(
        class DataFetcher {
            async fetchData() {
                return 42;
            }
        }

        const fetcher = new DataFetcher();
        const promise = fetcher.fetchData();
        promise;
    )", Value::Type::kObject); // async方法返回Promise对象
}

TEST_F(ClassAdvancedIntegrationTest, AsyncMethodWithAwait) {
    // 测试async方法中使用await
    AssertEq(R"(
        class Service {
            async getData() {
                return 100;
            }

            async processData() {
                const data = await this.getData();
                return data * 2;
            }
        }

        const service = new Service();
        const promise = service.processData();
        promise;
    )", Value::Type::kObject);
}

TEST_F(ClassAdvancedIntegrationTest, MultipleAsyncMethods) {
    // 测试多个async方法
    AssertEq(R"(
        class API {
            async fetchUser() {
                return { id: 1, name: 'Alice' };
            }

            async fetchPosts() {
                return [1, 2, 3];
            }
        }

        const api = new API();
        const p1 = api.fetchUser();
        const p2 = api.fetchPosts();
        p1;
    )", Value::Type::kObject);
}

TEST_F(ClassAdvancedIntegrationTest, StaticAsyncMethod) {
    // 测试静态async方法
    AssertEq(R"(
        class Util {
            static async fetchConfig() {
                return { debug: true, version: '1.0' };
            }
        }

        const promise = Util.fetchConfig();
        promise;
    )", Value::Type::kObject);
}

TEST_F(ClassAdvancedIntegrationTest, AsyncMethodInClassWithFields) {
    // 测试带有字段的类中的async方法
    AssertEq(R"(
        class RequestHandler {
            baseUrl = 'https://api.example.com';

            async request(endpoint) {
                return this.baseUrl + endpoint;
            }
        }

        const handler = new RequestHandler();
        const promise = handler.request('/users');
        promise;
    )", Value::Type::kObject);
}

// ==================== Generator方法 ====================

TEST_F(ClassAdvancedIntegrationTest, GeneratorMethodBasic) {
    // 测试基本的generator方法
    AssertEq(R"(
        class Sequence {
            *generateNumbers() {
                yield 1;
                yield 2;
                yield 3;
            }
        }

        const seq = new Sequence();
        const gen = seq.generateNumbers();
        gen.next().value;
    )", Value(1));
}

TEST_F(ClassAdvancedIntegrationTest, GeneratorMethodWithLoop) {
    // 测试带循环的generator方法
    AssertEq(R"(
        class Counter {
            *countTo(max) {
                for (let i = 1; i <= max; i += 1) {
                    yield i;
                }
            }
        }

        const counter = new Counter();
        const gen = counter.countTo(5);
        let sum = 0;
        for (let i = 0; i < 5; i += 1) {
            sum += gen.next().value;
        }
        sum;
    )", Value(15)); // 1+2+3+4+5 = 15
}

TEST_F(ClassAdvancedIntegrationTest, GeneratorMethodWithState) {
    // 测试generator方法维护状态
    AssertEq(R"(
        class FibonacciGenerator {
            *fibonacci() {
                let prev = 0;
                let curr = 1;
                while (true) {
                    yield curr;
                    const temp = prev + curr;
                    prev = curr;
                    curr = temp;
                    if (curr > 100) {
                        break;
                    }
                }
            }
        }

        const fib = new FibonacciGenerator();
        const gen = fib.fibonacci();
        let result = 0;
        for (let i = 0; i < 10; i += 1) {
            const r = gen.next();
            if (r.done) {
                break;
            }
            result = r.value;
        }
        result;
    )", Value(89));
}

TEST_F(ClassAdvancedIntegrationTest, StaticGeneratorMethod) {
    // 测试静态generator方法
    AssertEq(R"(
        class NumberUtil {
            static *range(start, end) {
                for (let i = start; i < end; i += 1) {
                    yield i;
                }
            }
        }

        const gen = NumberUtil.range(5, 10);
        let sum = 0;
        for (let i = 0; i < 5; i += 1) {
            sum += gen.next().value;
        }
        sum;
    )", Value(35)); // 5+6+7+8+9 = 35
}

TEST_F(ClassAdvancedIntegrationTest, GeneratorMethodWithThis) {
    // 测试generator方法访问this
    AssertEq(R"(
        class GeneratorWithState {
            start = 10;
            step = 5;

            *generate() {
                let current = this.start;
                while (current < 50) {
                    yield current;
                    current += this.step;
                }
            }
        }

        const g = new GeneratorWithState();
        const gen = g.generate();
        let sum = 0;
        for (let i = 0; i < 5; i += 1) {
            const r = gen.next();
            if (r.done) break;
            sum += r.value;
        }
        sum;
    )", Value(70)); // 10+15+20+25 = 70
}

// ==================== 混合特性测试 ====================

TEST_F(ClassAdvancedIntegrationTest, AsyncAndGeneratorTogether) {
    // 测试async和generator方法在同一类中
    AssertEq(R"(
        class MixedClass {
            data = [1, 2, 3];

            *generator() {
                for (let i = 0; i < this.data.length; i += 1) {
                    yield this.data[i];
                }
            }

            async asyncMethod() {
                return 'async result';
            }
        }

        const obj = new MixedClass();
        const gen = obj.generator();
        let sum = 0;
        for (let i = 0; i < 3; i += 1) {
            sum += gen.next().value;
        }
        sum;
    )", Value(6));
}

TEST_F(ClassAdvancedIntegrationTest, StaticAsyncAndInstanceMethods) {
    // 测试静态async方法和实例方法混合
    AssertEq(R"(
        class APIClient {
            static baseUrl = 'https://api.example.com';

            constructor(token) {
                this.token = token;
            }

            getAuthHeader() {
                return 'Bearer ' + this.token;
            }

            static async getVersion() {
                return '1.0.0';
            }
        }

        const client = new APIClient('secret');
        const auth = client.getAuthHeader();
        const promise = APIClient.getVersion();
        auth;
    )", Value("Bearer secret"));
}

// ==================== 计算属性名高级用法 ====================

TEST_F(ClassAdvancedIntegrationTest, ComputedPropertyWithExpression) {
    // 测试表达式作为计算属性名
    AssertEq(R"(
        const prefix = 'get';
        const suffix = 'Value';

        class MyClass {
            _value = 42;

            [prefix + suffix]() {
                return this._value;
            }

            ['set' + suffix](value) {
                this._value = value;
            }
        }

        const obj = new MyClass();
        obj.setValue(100);
        obj.getValue();
    )", Value(100));
}

TEST_F(ClassAdvancedIntegrationTest, MultipleComputedProperties) {
    // 测试多个计算属性
    AssertEq(R"(
        const methods = ['add', 'multiply'];

        class Calculator {
            result = 0;

            [methods[0]](a, b) {
                return a + b;
            }

            [methods[1]](a, b) {
                return a * b;
            }
        }

        const calc = new Calculator();
        calc.add(5, 3) + calc.multiply(2, 4);
    )", Value(16)); // 8 + 8 = 16
}

TEST_F(ClassAdvancedIntegrationTest, ComputedStaticGetterSetter) {
    // 测试计算属性的静态getter/setter
    AssertEq(R"(
        const propName = 'config';

        class ConfigManager {
            static _data = {};

            static get [propName]() {
                return ConfigManager._data;
            }

            static set [propName](value) {
                ConfigManager._data = value;
            }
        }

        ConfigManager.config = { key: 'value' };
        ConfigManager.config.key;
    )", Value("value"));
}

// ==================== 字段高级特性 ====================

TEST_F(ClassAdvancedIntegrationTest, FieldWithComplexInitialization) {
    // 测试字段复杂初始化
    AssertEq(R"(
        class ComplexFields {
            data = [1, 2, 3, 4, 5];
            sum = this.data.reduce((acc, val) => acc + val, 0);
            config = { min: 0, max: 100 };

            getSum() {
                return this.sum;
            }
        }

        const obj = new ComplexFields();
        obj.getSum();
    )", Value(15));
}

TEST_F(ClassAdvancedIntegrationTest, FieldReferencingOtherFields) {
    // 测试字段引用其他字段
    AssertEq(R"(
        class FieldReference {
            x = 10;
            y = 20;
            sum = this.x + this.y;

            getTotal() {
                return this.sum;
            }
        }

        const obj = new FieldReference();
        obj.getTotal();
    )", Value(30));
}

TEST_F(ClassAdvancedIntegrationTest, StaticFieldsWithObjects) {
    // 测试静态字段存储对象
    AssertEq(R"(
        class Config {
            static settings = {
                debug: true,
                version: '1.0.0',
                features: ['feature1', 'feature2']
            };

            static getFeatureCount() {
                return Config.settings.features.length;
            }
        }

        Config.getFeatureCount();
    )", Value(2));
}

// ==================== 边界情况和错误处理 ====================

TEST_F(ClassAdvancedIntegrationTest, EmptyClassWithOnlyAsyncMethod) {
    // 测试只有async方法的空类
    AssertEq(R"(
        class OnlyAsync {
            async doSomething() {
                return 42;
            }
        }

        const obj = new OnlyAsync();
        const promise = obj.doSomething();
        promise;
    )", Value::Type::kObject);
}

TEST_F(ClassAdvancedIntegrationTest, ClassWithOnlyGenerator) {
    // 测试只有generator方法的类
    AssertEq(R"(
        class OnlyGenerator {
            *generate() {
                yield 1;
                yield 2;
                yield 3;
            }
        }

        const obj = new OnlyGenerator();
        const gen = obj.generate();
        gen.next().value;
    )", Value(1));
}

TEST_F(ClassAdvancedIntegrationTest, MultipleConstructorsError) {
    // 测试多个构造函数的行为（只应最后一个生效）
    AssertEq(R"(
        class Test {
            constructor() {
                this.value = 1;
            }

            constructor() {
                this.value = 2;
            }
        }

        const obj = new Test();
        obj.value;
    )", Value(2));
}

TEST_F(ClassAdvancedIntegrationTest, ClassExpressionWithAsyncMethod) {
    // 测试类表达式中的async方法
    AssertEq(R"(
        const MyClass = class {
            async fetchData() {
                return { data: 'test' };
            }
        };

        const obj = new MyClass();
        const promise = obj.fetchData();
        promise;
    )", Value::Type::kObject);
}

TEST_F(ClassAdvancedIntegrationTest, ClassExpressionWithGenerator) {
    // 测试类表达式中的generator方法
    AssertEq(R"(
        const GeneratorClass = class {
            *sequence() {
                yield 1;
                yield 2;
                yield 3;
            }
        };

        const obj = new GeneratorClass();
        const gen = obj.sequence();
        gen.next().value;
    )", Value(1));
}

// ==================== 实际应用场景 ====================

TEST_F(ClassAdvancedIntegrationTest, AsyncIteratorPattern) {
    // 测试异步迭代器模式
    AssertEq(R"(
        class AsyncCollection {
            items = [1, 2, 3, 4, 5];

            *getItems() {
                for (let i = 0; i < this.items.length; i += 1) {
                    yield this.items[i];
                }
            }

            getFilteredItems(predicate) {
                const results = [];
                const gen = this.getItems();
                let item = gen.next();
                while (!item.done) {
                    if (predicate(item.value)) {
                        results.push(item.value);
                    }
                    item = gen.next();
                }
                return results;
            }
        }

        const collection = new AsyncCollection();
        const filtered = collection.getFilteredItems(x => x > 2);
        filtered.length;
    )", Value(3));
}

TEST_F(ClassAdvancedIntegrationTest, StateMachineWithGenerator) {
    // 测试使用generator实现状态机
    AssertEq(R"(
        class StateMachine {
            currentState = 'idle';

            *transition(action) {
                while (true) {
                    switch (this.currentState) {
                        case 'idle':
                            this.currentState = 'running';
                            yield 'started';
                            break;
                        case 'running':
                            this.currentState = 'paused';
                            yield 'paused';
                            break;
                        case 'paused':
                            this.currentState = 'stopped';
                            yield 'stopped';
                            return;
                        default:
                            return;
                    }
                }
            }
        }

        const sm = new StateMachine();
        const gen = sm.transition();
        const r1 = gen.next().value;
        const r2 = gen.next().value;
        const r3 = gen.next().value;
        r3;
    )", Value("stopped"));
}

TEST_F(ClassAdvancedIntegrationTest, ReactiveStorePattern) {
    // 测试响应式Store模式
    AssertEq(R"(
        class Store {
            _state = { count: 0 };
            listeners = [];

            getState() {
                return this._state;
            }

            setState(newState) {
                this._state = newState;
                this.notify();
            }

            subscribe(listener) {
                this.listeners.push(listener);
            }

            notify() {
                for (let i = 0; i < this.listeners.length; i += 1) {
                    this.listeners[i](this._state);
                }
            }
        }

        const store = new Store();
        let lastState = null;

        store.subscribe(function(state) {
            lastState = state;
        });

        store.setState({ count: 5 });
        lastState.count;
    )", Value(5));
}

} // namespace mjs::test
