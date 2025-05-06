

let promise = new Promise(function(resolve, reject) {
    console.log("constructor");
    resolve(1);
});

promise.then(function(val) {
    console.log("promise1!! ", val);
    return 2;
}).then(function(val) {
    console.log("promise2!! ", val);
});

// promise.resolve(1);
// promise.resolve();


async function asyncfunc2(par) {
    console.log("asyncfunc2 await begin!!");
    let res = await new Promise(function(resolve) {
        resolve(6666);
    });
    console.log("asyncfunc2 await ok1!!", res);
    res = await new Promise(function(resolve) {
        resolve(7777);
    });
    // throw "abc";
    console.log("asyncfunc2 await ok2!!", res);
    return par;
}

async function asyncfunc() {
    console.log("asyncfunc await begin!!");
    let res = await new Promise(function(resolve) {
        resolve(666);
    });
    console.log("asyncfunc await ok1!!", res);
    res = await new Promise(function(resolve) {
        resolve(777);
    });
    console.log("asyncfunc await ok2!!", res);
    res = await asyncfunc2(114514);
    console.log("asyncfunc await ok3!!", res);
}

let res_promise1 = asyncfunc();
let res_promise66 = asyncfunc();

let res_promise2 = asyncfunc2(123);


// Promise.resolve(res_promise2);



let p = Promise.resolve(42);
p.then(function(val) { console.log(val); });





//anonymousfunc();
//anonymousfunc();





