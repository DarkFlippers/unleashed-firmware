let sampleText = "Hello, World!";

let lengthOfText = "Length of text: " + sampleText.length.toString();
print(lengthOfText);

let start = 7;
let end = 12;
let substringResult = sampleText.slice(start, end);
print(substringResult);

let searchStr = "World";
let result2 = sampleText.indexOf(searchStr).toString();
print(result2);

let upperCaseText = "Text in upper case: " + sampleText.toUpperCase();
print(upperCaseText);

let lowerCaseText = "Text in lower case: " + sampleText.toLowerCase();
print(lowerCaseText);
