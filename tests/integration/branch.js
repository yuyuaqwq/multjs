/* sb
sbs
b */

let arr = [3123, 231, "dawd", ["sbsb", 666]];
// let index = arr[100];

// let obj = {fake:1243, sbsb:666};

let qq = 1 + 2 - 3 + 4 - 5;
console.log(qq); // -1

console.log(-(1.5864 + 12) * 666); // -9,048.5424

let a = 123;
let b = 233;
a = b = a + b + 4444;

console.log(a, b); // 48004800

function tick() {
    return 666;
}

let t = tick();
let i = 0;
while (i < 10000) {
    i = i + 1.21;
}
console.log("i:", i, "    ", "tick:", tick() - t, "ms"); // 10000.649999998921

i = 0;
while (i < 100) {
    i = i + 1;
    if (i == 50) {
        console.log("i:", 666);
    }
    if (i > 80) {
        let j = 0;
        while (j < 10) {
            console.log("j:", j, "i", i);
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
    console.log("i:", i);
}
console.log("i:", i);


if (a == 2) {
    console.log("a is 2");
}
else {
    if (a == 3) {
        console.log("a is 3");
    }
    else if (a == 9) {
        console.log("a is 9");
    }
    else if (a == 4800) {
        console.log("a is 4800");
    }
    else {
        console.log("a is not 3, 9, or 100");
    }

    console.log("a is not 2");
}

let qvqvq = 10000000;
{
    a = 111;
    let q = "333";
}

function add(a, b) {
    a = a + b;
    console.log("add call");
    console.log(a + qvqvq);

    function hanhanyufan() {
        console.log("hanhanyufan", "qvqvq", qvqvq);
        console.log(qvqvq);
        return 999;
    }

    a = hanhanyufan();

    console.log("a:", a);
}

console.log("hello world!");

add(666, a, console.log);

let c = "abc";