
function* gen() {
    yield 1;
    yield 2;
    yield 3;
}

let g = gen();
println(g.next().value); // 1
println(g.next().done); // 2
println(g.next().value); // 3
println(g.next().value); // undefined
println(g.next().done); // undefined


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