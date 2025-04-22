let anonymousfunc = function() {
    try {
        try {
            let i = 0;
            while (i < 5) {
                i = i + 1;
                try {
                    try {
                        println("inner try while try2");
                        continue;
                        break;
                    }
                    finally {
                        println("inner try while finally2");
                        throw "sbsb666";
                        break;
                    }
                }
                finally {
                    println("inner try while finally");
                    break;
                }
            }
            {
                return 123321;
            }
        }
        finally {
            println("inner finally2");
            return 8888;
        }
        println("inner try");
        throw "inner error";
    }
    // catch (e) {
    //     println("inner catch:", e);
    // }
    finally {
        // println("inner finally");
        return 6666;
    }
};

let res = undefined;
try {
    res = anonymousfunc();
}
catch (e) {
    println("call catch:", e);
}
finally {
    println("call finally");
}
println("res:", res);


try {
    println("outer try");
    try {
        println("inner try");
        throw "inner error";
    }
    catch (e) {
        try {
            println("inner catch:", e);
            throw "new error from catch"; // 重新抛出
        }
        catch (e) {
            println("inner2 catch:", e);
            throw "new error from inner catch"; // 重新抛出
        }
        finally {
            println("inner2 finally");
            throw "sbsb";
        }
    }
    finally {
        println("inner finally");
    }
}
catch (e) {
    println("outer catch:", e);
}
finally {
    println("dawdawd outer finally");
    // throw "2b";
}
