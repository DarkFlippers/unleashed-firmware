let arr_1 = Uint8Array([0, 1, 2, 3, 4, 5, 6, 7, 8, 9]);
print("len =", arr_1.buffer.byteLength);

let arr_2 = Uint8Array(arr_1.buffer.slice(2, 6));
print("slice len =", arr_2.buffer.byteLength);
for (let i = 0; i < arr_2.buffer.byteLength; i++) {
    print(arr_2[i]);
}
