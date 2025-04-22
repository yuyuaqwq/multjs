
let constvar = { a:{a:"123"} };
constvar = "456";

function tt() {
    println("tt:", 100);
    return 100;
}
let obj = {};
obj.sb = tt;
obj.sb = obj.sb2 = obj.sb();
println(obj.sb, " ", obj.sb2);

obj.sb = {};
obj.sb.sb2 = tt;
obj.sb.sb2();


let arr = [1, 2, 3];
println(arr[0]);
println(arr[1]);
arr[1] = 666;
println(arr[1]);

arr[1] = [6, "7", "8"];
println(arr[1][2]);
arr[1][2] = 888;
println(arr[1][2]);

arr[1] = tt;
arr[1]();

let sb = 1;
sb.emm = 666;
println(sb.emm);

println("abc" + 123);