import * as module2 from './test_module2.js';

function a() {
    println("module1 func a.", module2.PI, module2.square(module2.PI));
}

println("module1 loaded.", module2.PI, module2.square(module2.PI));

a();