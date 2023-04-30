import { Transform } from "stream";
import nodeGyp from "node-gyp-build";
const addon = nodeGyp(process.cwd());

export interface Bzip2Options {
  level?: 1|2|3|4|5|6|7|8|9;
  workFactor?: number;
  verbosity?: number;
}

export function CompressStream(options?: Bzip2Options): Transform {
  const str = new Transform({autoDestroy: true, emitClose: true, defaultEncoding: "binary", decodeStrings: true, encoding: "binary"});
  return addon.Compress(str, {
    level: Math.max(1, Math.min(9, options?.level || 0)),
    verbosity: Math.max(0, Math.min(256, options?.verbosity || 0)),
    workFactor: Math.max(0, Math.min(250, options?.workFactor || 0)),
  });
}