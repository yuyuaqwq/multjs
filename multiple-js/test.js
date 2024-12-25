/* sb
sbs
b */

// let arr = [3123, 231, "dawd", ["sbsb", 666]];
// let index = arr[100];

// let qq = 1 + 2 - 3 + 4 - 5;
// println(qq);

// println(-(1.5864 + 12) * 666);

let a = 123;
let b = 233;
a = b = a + b + 4444;

println(a, b);

let t = tick();
let i = 0;
while (i < 10000000) {
    i = i + 1.21;
}
println("i:", i, "    ", "tick:", tick() - t, "ms");

i = 0;
while (i < 100) {
    i = i + 1;
    if (i == 50) {
        println("i:", 666);
    }
    if (i > 80) {
        let j = 0;
        while (j < 10) {
            println("j:", j);
            j = j + 1;
            if (j > 5) {
                break;
            }
        }
        continue;
    }
    else if (i >= 90) {
        break;
    }
    println("i:", i);
}
println("i:", i);


if (a == 2) {
    println("a is 2");
}
else {
    if (a == 3) {
        println("a is 3");
    }
    else if (a == 9) {
        println("a is 9");
    }
    else if (a == 4800) {
        println("a is 4800");
    }
    else {
        println("a is not 3, 9, or 100");
    }

    println("a is not 2");
}

let qvqvq = 10000000;
{
    a = 111;
    let q = "333";
}

function add(a, b) {
    a = a + b;
    println("add call");
    println(a + qvqvq);

    function hanhanyufan() {
        println("hanhanyufan", "qvqvq", qvqvq);
        println(qvqvq);
        return 999;
    }

    a = hanhanyufan();

    println("a:", a);
}

println("hello world!");

add(666, a, println);

let c = "abc";