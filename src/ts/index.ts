import { Transform } from "stream";
import nodeGyp from "node-gyp-build";
const addon = nodeGyp(process.cwd());

export function CompressStream() {
  const str = new Transform();
  addon.compress(str);
  return str;
}

export function DescompressStream() {
  const str = new Transform();
  addon.compress(str);
  return str;
}