import * as module2 from './test_module2.js';

async function a() {
    println("module1 func a.", module2.PI, module2.square(module2.PI));
    let mod = await import('./test_module2.js');
    println("module1 func a await ok.", mod.PI, mod.square(mod.PI));
}

a();

println("module1 loaded.", module2.PI, module2.square(module2.PI));

