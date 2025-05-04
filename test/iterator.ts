const iterableObject = {
    data: [1, 2, 3],
    [Symbol.iterator]: function() {
        let index = 0;
        return {
            next: () => {
                if (index < this.data.length) {
                    return { value: this.data[index++], done: false };
                } else {
                    return { value: undefined, done: true };
                }
            }
        };
    }
};

// Using for...of
for (const value of iterableObject) {
    println(value);
}
// Output:
// 1
// 2
// 3