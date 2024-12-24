let a = 123;
let b = 233;
a = a + b + 4444;

let t = tick();
let i = 0;
while (i < 10000000) {
    i = i + 1;
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
            println("iii:", i);
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
    println("a 是 2");
}
else {
    if (a == 3) {
        println("a 是 3");
    }
    else if (a == 9) {
        println("a 是 9");
    }
    else if (a == 4800) {
        println("a 是 4800");
    }
    else {
        println("a 不是 3和9和100");
    }

    println("a 不是 2");
}

let qvqvq = 10000000;
{
    a = 111;
    let q = "333";
}

function add(a, b) {
    a = a + b;
    println("add啦");
    println(a + qvqvq);

    function hanhanyufan() {
        println("啊啊啊啊", "是雨凡", qvqvq);
        println(qvqvq);
        return 999;
    }

    a = hanhanyufan();

    println("a的结果是：", a);
}

println("hello world!");

add(666, a, println);

let c = "abc";