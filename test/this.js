let f = function () {
    // console.log(this.x);
    println(this.x);
};
  
let x = 1;
let obj = {
    f: f,
    x: 2,
};
  
// 单独执行
f(); // 1

// obj 环境执行
obj.f(); // 2