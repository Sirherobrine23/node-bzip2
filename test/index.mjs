import { Transform } from "stream";
import nodeGyp from "node-gyp-build";
const addon = nodeGyp(process.cwd());
console.log("Starting");
const str = addon.Compress(new Transform({autoDestroy: true, emitClose: true, defaultEncoding: "binary", decodeStrings: true, encoding: "binary"}));
str.on("error", console.error).on("data", console.log.bind(this, "Data: %O")).on("close", () => console.log("Close"));
console.log("Writing");
str.write(Buffer.from([0x12]));