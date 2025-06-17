let cc = 1000;

function add(a, b) {
    console.log("a:", a, ", b:", b);
    a = a + b;

    function bbb(c, d, e) {
        console.log("c:", c, ", d:", d, ", e:", e, ", cc:", cc);
        let f = c + d + e;
        return f;
    }
    // console.log("add call");
    // console.log(a);
    let c = bbb(a, a, b, console.log, "faf", "dwadwad", "dwad");
    cc = 2000;
    c = bbb(a, a, b, console.log, "faf", "dwadwad", "dwad");

    return c;
}

function tt() {
    console.log("sbsb:", 1);
    let sbsb = 50;
    function qqq() {
        sbsb = sbsb + 100;
        console.log("qqq:", sbsb);
        cc = cc + 5;
        console.log("cc:", cc);
    }
    return qqq;
}

console.log("hello world!");

let a = 1;
let b = add(666, a, "sbsb");
console.log("b:", b);

let tt_1 = tt();
tt_1();
tt_1();

let tt_2 = tt();
tt_2();
tt_2();

let tt_3 = tt_2;
tt_3();

tt_2();

