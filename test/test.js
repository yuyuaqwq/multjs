function add(a, b) {
    a = a + b;

    function bbb(c, d, e) {
        let f = c + d + e;
        return f;
    }
    // println("add call");
    // println(a);
    let c = bbb(a, a, b);
    return c;
}

// println("hello world!");

let a = 1;
let b = add(666, a);
println("b", b);