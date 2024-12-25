/* sb
sbs
b */

println(-(1.5864 + 12) * 666);

let a = 123;
let b = 233;
a = a + b + 4444;

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
    println("a �� 2");
}
else {
    if (a == 3) {
        println("a �� 3");
    }
    else if (a == 9) {
        println("a �� 9");
    }
    else if (a == 4800) {
        println("a �� 4800");
    }
    else {
        println("a ���� 3��9��100");
    }

    println("a ���� 2");
}

let qvqvq = 10000000;
{
    a = 111;
    let q = "333";
}

function add(a, b) {
    a = a + b;
    println("add��");
    println(a + qvqvq);

    function hanhanyufan() {
        println("��������", "���귲", qvqvq);
        println(qvqvq);
        return 999;
    }

    a = hanhanyufan();

    println("a�Ľ���ǣ�", a);
}

println("hello world!");

add(666, a, println);

let c = "abc";