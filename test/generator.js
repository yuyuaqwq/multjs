
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

