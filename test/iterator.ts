const simpleIterator = {
    data: [1, 2, 3],
    index: 0,
    next: function() {
        if (this.index < this.data.length) {
            return { value: this.data[this.index++], done: false };
        }
        else {
            return { value: undefined, done: true };
        }
    }
};

println(simpleIterator.next()); // { value: 1, done: false }
println(simpleIterator.next()); // { value: 2, done: false }