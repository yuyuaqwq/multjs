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

