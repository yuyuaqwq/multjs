// // 基本箭头函数测试
// const basicArrow = () => "Hello World";
// println(basicArrow()); // 输出: Hello World

// // 带参数的箭头函数
// const add = (a, b) => a + b;
// println(add(1, 2)); // 输出: 3

// const setTimeout = (func, time) => func();

// // 异步箭头函数
// const asyncArrow = async () => {
//     return new Promise(resolve => {
//         setTimeout(() => resolve("Async Hello"), 1000);
//     });
// };
// asyncArrow().then(println); // 输出: Async Hello

// // 块语句体的箭头函数
// const blockArrow = (x) => {
//     const y = x * 2;
//     return y + 1;
// };
// println(blockArrow(3)); // 输出: 7

// // 表达式体的箭头函数
// const exprArrow = x => x * x;
// println(exprArrow(4)); // 输出: 16

// // 箭头函数作为回调
// // const numbers = [1, 2, 3, 4, 5];
// // const doubled = numbers.map(x => x * 2);
// // println(doubled); // 输出: [2, 4, 6, 8, 10]

// // 箭头函数中的this绑定
// const obj = {
//     name: "Test",
//     getName: function() {
//         setTimeout(() => {
//             println(this.name); // 输出: Test
//         }, 100);
//     }
// };
// obj.getName(); 

// Promise.resolve(Promise.resolve(Promise.resolve(42))).then(value => println(value)); // 应该输出42

let console = {
    log: function(info) {
        println(info);
    }
};

// Promise.resolve(42)
//   .then(v => {
//     println(v); // 应输出42
//     return v + 1;
//   })
//   .then(v => println(v)); // 应输出43

//   const p = Promise.resolve(
//     Promise.resolve(
//       Promise.resolve("Final Value")
//     )
//   );
  
//   p.then((val) => {
//     console.log(val); // 输出 "Final Value"（递归解包到最内层）
//   });



let r1 = undefined;
let r2 = undefined;

let p1 = new Promise((resolve)=>{
    r1 = resolve;
}).then(value=>{
    console.log(value);
});

r1(new Promise((resolve)=>{
    r2 = resolve;
}));

r2('hello');