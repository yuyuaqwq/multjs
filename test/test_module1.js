import * as module2 from './test_module2.js';

// async function a() {
//     println("module1 func a.", module2.PI, module2.square(module2.PI));
//     let mod = await import('./test_module2.js');
//     println("module1 func a await ok.", mod.PI, mod.square(mod.PI));
// }

// a();

// println("module1 loaded.", module2.PI, module2.square(module2.PI));

// let c = 0;
// label:
// while (c <= 10) {
//     while (c <= 10) {
//         c = c + 1;
//         println("c", c);
//         continue label;
//     }
//     println("c2", c);
// }

// c = 0;
// label:
// while (c <= 10) {
//     while (c <= 10) {
//         c = c + 1;
//         println("c", c);
//         break label;
//     }
//     println("c2", c);
// }

label:
for (let i = 0; i < 10; i = i + 1) {
    for (let j = 0; j < 10; j = j + 1) {
        println("i", i, "j", j);
        continue label;
    }
    println("i", i);
}

// label:
// while (c <= 10) {
//     while (c <= 10) {
//         c = c + 1;
//         break label;
//     }
// }