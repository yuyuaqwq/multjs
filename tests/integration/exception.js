let anonymousfunc = function() {
    try {
        try {
            let i = 0;
            while (i < 5) {
                i = i + 1;
                try {
                    try {
                        console.log("inner try while try2");
                        continue;
                        break;
                    }
                    finally {
                        console.log("inner try while finally2");
                        throw "sbsb666";
                        break;
                    }
                }
                finally {
                    console.log("inner try while finally");
                    break;
                }
            }
            {
                return 123321;
            }
        }
        finally {
            console.log("inner finally2");
            return 8888;
        }
        console.log("inner try");
        throw "inner error";
    }
    // catch (e) {
    //     console.log("inner catch:", e);
    // }
    finally {
        // console.log("inner finally");
        return 6666;
    }
};

let res = undefined;
try {
    res = anonymousfunc();
}
catch (e) {
    console.log("call catch:", e);
}
finally {
    console.log("call finally");
}
console.log("res:", res);


try {
    console.log("outer try");
    try {
        console.log("inner try");
        throw "inner error";
    }
    catch (e) {
        try {
            console.log("inner catch:", e);
            throw "new error from catch"; // 重新抛出
        }
        catch (e) {
            console.log("inner2 catch:", e);
            throw "new error from inner catch"; // 重新抛出
        }
        finally {
            console.log("inner2 finally");
            throw "sbsb";
        }
    }
    finally {
        console.log("inner finally");
    }
}
catch (e) {
    console.log("outer catch:", e);
}
finally {
    console.log("dawdawd outer finally");
    // throw "2b";
}
