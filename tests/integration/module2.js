import * as module1 from './module1.js';

export const PI = 3.14159;
export function square(x) { return x * x; }

console.log("module2 loaded.", PI, square(PI));
