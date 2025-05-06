import * as module2 from './module2.js';

async function a() {
    console.log("module1 func a.", module2.PI, module2.square(module2.PI));
    let mod = await import('./module2.js');
    console.log("module1 func a await ok.", mod.PI, mod.square(mod.PI));
}

a();

console.log("module1 loaded.", module2.PI, module2.square(module2.PI));

let c = 0;
label:
while (c <= 10) {
    while (c <= 10) {
        c = c + 1;
        console.log("c", c);
        continue label;
    }
    console.log("c2", c);
}

c = 0;
label:
while (c <= 10) {
    while (c <= 10) {
        c = c + 1;
        console.log("c", c);
        break label;
    }
    console.log("c2", c);
}

label:
for (let i = 0; i < 10; i = i + 1) {
    for (let j = 0; j < 10; j = j + 1) {
        console.log("i", i, "j", j);
        continue label;
    }
    console.log("i", i);
}

// label:
// while (c <= 10) {
//     while (c <= 10) {
//         c = c + 1;
//         break label;
//     }
// }