
let number = 123;

let constvar = { a:{a:"123"} };
constvar = "456";



function tt(a, b) {
    console.log("tt:", 100, a, b);
    return 100;
}
tt("sb", "666");

let obj = { sb: tt };
obj.sb = tt;
obj.sb = obj.sb2 = obj.sb("dd", "qq");
console.log(obj.sb, " ", obj.sb2);

console.log(obj["sb"]);
console.log(obj["s" + "b"]);
console.log("???", obj["s" + "b" + "2"]);
console.log("???", obj["s" + "b" + "3"]);

obj.sb = {};
obj.sb.sb2 = tt;
obj.sb.sb2("aa", "bb");


let arr = [1, 2, "123"];
console.log(arr[0]);
console.log(arr[1]);
arr[1] = 666;
console.log(arr[1]);

// arr[50] = 1;

arr[1] = [6, "7", "8"];
console.log(arr[1][2]);
arr[1][2] = 888;
console.log(arr[1][2]);

arr[1] = tt;
arr[1]("sha", "niao");

let sb = 1;
sb.emm = 666;
console.log(sb.emm);

console.log("abc" + 123);