import { Transform } from "stream";
import nodeGyp from "node-gyp-build";
const addon = nodeGyp(process.cwd());

export interface Bzip2Options {
  level?: 1|2|3|4|5|6|7|8|9;
}

export function CompressStream(options?: Bzip2Options): Transform {
  const str = new Transform();
  // Call node addon
  return addon.Compress(options||{}, str);
}