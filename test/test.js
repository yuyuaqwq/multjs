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

// println("hello world!");

let a = 1;
let b = add(666, a, "sbsb");
println("b", b);