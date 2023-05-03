import { randomBytes, randomInt } from "crypto";
import { Transform, finished } from "stream";
import nodeGyp from "node-gyp-build";
const addon = nodeGyp(process.cwd());

/**
 *
 * @param {{ level?: 1|2|3|4|5|6|7|8|9; workFactor?: number; verbosity?: number; }} options
 *
 * @return {Transform}
 */
export function CompressStream(options) {
  if (!options) options = {};
  return addon.Compress(new Transform({}), {
    level: Math.max(1, Math.min(9, options?.level || 0)),
    verbosity: Math.max(0, Math.min(256, options?.verbosity || 0)),
    workFactor: Math.max(0, Math.min(256, options?.workFactor || 0)),
  });
}

const str = CompressStream({
  level: 1,
  verbosity: 0,
  workFactor: 0
});

str.on("error", console.error).on("data", (chunk) => console.log("Compressed data:", chunk));
const Size = randomInt(8, 256);
const data = randomBytes(Size);
console.log("Writing random, byteLength: %f, Random: %f", data.byteLength, Size);
str.end(data);