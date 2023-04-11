import { greetAsync, CustomObject } from "@nodepp/example";

async function main() {
  const greeting = await greetAsync("Node");
  console.log(greeting);
}

main();

const obj = new CustomObject(10);

console.log(obj.getValue());
obj.add(5);
console.log(obj.getValue());
obj.multiply(2);
console.log(obj.getValue());
