import { randomBytes, randomInt } from "crypto";
import { Transform, finished } from "stream";
import nodeGyp from "node-gyp-build";
import SegfaultHandler from "segfault-handler";
SegfaultHandler.registerHandler("crash.log");
const addon = nodeGyp(process.cwd());

/**
 *
 * @param {{ level?: 1|2|3|4|5|6|7|8|9; workFactor?: number; verbosity?: number; }} options
 * @returns {Transform}
 */
export function CompressStream(options) {
  if (!options) options = {};
  return addon.Compress(new Transform({autoDestroy: true, emitClose: true}), {
    level: Math.max(1, Math.min(9, options?.level || 0)),
    verbosity: Math.max(0, Math.min(256, options?.verbosity || 0)),
    workFactor: Math.max(0, Math.min(256, options?.workFactor || 0)),
  });
}

const str = CompressStream({
  level: 1,
  verbosity: 1,
  workFactor: 10
});

const compressed = [];
str.on("error", console.error).on("data", buf => {
  compressed.push(buf);
});
console.log("Writing");
const data = randomBytes(randomInt(1, 256));
console.log("Initial data size: %f", Buffer.byteLength(data));
str.end(data);
finished(str, () => console.log("Closes finished, end with size: %f", Buffer.byteLength(Buffer.concat(compressed))));