let p = Promise.resolve(42);
p.then(function(val) { println(val); });



let anonymousfunc = function() {
    println("anonymousfunc");
    return;
};

anonymousfunc();
anonymousfunc();

let promise = new Promise();

promise.then(function(val) {
    println("promise!! ", val);
    return;
});

promise.resolve(1);
promise.resolve();


async function asyncfunc() {
    let res = await 123;
}

let res_promise = asyncfunc();

function* gen(a, b, c) {
    let ttt = this;
    let ada = "emm";
    let res1 = yield a + ada;
    println(res1, " ", ada);
    let bbb = res1 + 1000;
    let res2 = yield b + bbb;
    println(res2, " ", bbb);
    let ccc = res2 + 2000;
    let res3 = yield c + ccc;
    println(res3, " ", ccc);
    let ddd = res3 + 3000;
    return 666 + ddd;
}

let g = gen(10, 20, 30);
let n1 = g.next(90000, 8);
println(n1.value, " ", n1.done);
let n2 = g.next(80000);
println(n2.value, " ", n2.done);
let n3 = g.next(70000);
println(n3.value, " ", n3.done);
let n4 = g.next(60000);
println(n4.value, " ", n4.done);
let n5 = g.next(50000);
println(n5.value, " ", n5.done);




let cc = 1000;

function add(a, b) {
    println("a:", a, ", b:", b);
    a = a + b;

    function bbb(c, d, e) {
        println("c:", c, ", d:", d, ", e:", e, ", cc:", cc);
        let f = c + d + e;
        return f;
    }
    // println("add call");
    // println(a);
    let c = bbb(a, a, b, println, "faf", "dwadwad", "dwad");
    cc = 2000;
    c = bbb(a, a, b, println, "faf", "dwadwad", "dwad");

    return c;
}

function tt() {
    println("sbsb:", 1);
    let sbsb = 50;
    function qqq() {
        sbsb = sbsb + 100;
        println("qqq:", sbsb);
        cc = cc + 5;
        println("cc:", cc);
    }
    return qqq;
}

println("hello world!");

let a = 1;
let b = add(666, a, "sbsb");
println("b:", b);

let tt_1 = tt();
tt_1();
tt_1();

let tt_2 = tt();
tt_2();
tt_2();

let tt_3 = tt_2;
tt_3();

tt_2();



// function tt() {
//     println("tt:", 100);
//     return 100;
// }
let obj = {};
obj.sb = tt;
obj.sb = obj.sb2 = obj.sb();
println(obj.sb, " ", obj.sb2);

obj.sb = {};
obj.sb.sb2 = tt;
obj.sb.sb2();


let arr = [1, 2, 3];
println(arr[0]);
println(arr["1"]);
arr[1] = 666;
println(arr[1]);

arr[1] = [6, "7", "8"];
println(arr["1"]["2"]);
println(arr[1][2]);
arr[1][2] = 888;
println(arr[1][2]);
println(arr["1"]["2"]);

arr["1"] = tt;
arr["1"]();

let sb = 1;
sb.emm = 666;
println(sb.emm);

println("abc" + 123);