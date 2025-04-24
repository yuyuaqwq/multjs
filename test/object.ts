
let number : number = 123;

let constvar = { a:{a:"123"} };
constvar = "456";

function tt(a : string, b : string) : number {
    println("tt:", 100, a, b);
    return 100;
}
tt("sb", "666");

let obj = { sb: tt };
obj.sb = tt;
obj.sb = obj.sb2 = obj.sb("dd", "qq");
println(obj.sb, " ", obj.sb2);

obj.sb = {};
obj.sb.sb2 = tt;
obj.sb.sb2("aa", "bb");


let arr = [1, 2, "123"];
println(arr[0]);
println(arr[1]);
arr[1] = 666;
println(arr[1]);

// arr[50] = 1;

arr[1] = [6, "7", "8"];
println(arr[1][2]);
arr[1][2] = 888;
println(arr[1][2]);

arr[1] = tt;
arr[1]("sha", "niao");

let sb = 1;
sb.emm = 666;
println(sb.emm);

println("abc" + 123);