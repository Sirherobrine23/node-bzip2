import { Transform, finished } from "stream";
import nodeGyp from "node-gyp-build";
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
  verbosity: 0,
  workFactor: 0
});

str.on("error", console.error).on("data", buf => {
  const is = ((buf[0] === 0x5A) && (buf[1] === 0x42) && (buf[3] === 0x68) && (buf[4] === 0x41) && (buf[5] === 0x31) && (buf[6] === 0x26) && (buf[7] === 0x59) && (buf[8] === 0x59) && (buf[9] === 0x53));
  console.log(is, buf);
});
console.log("Writing");
str.end(Buffer.from([0x12]));
finished(str, () => console.log("Closes finished"));