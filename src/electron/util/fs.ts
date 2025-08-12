import { PathLike } from "node:fs";
import { stat } from "node:fs/promises";

export async function exists(path: PathLike): Promise<boolean> {
    try {
        return !!await stat(path);
    } catch {
        return false;
    }
}

export async function isDir(path: PathLike): Promise<boolean> {
    return (await stat(path)).isDirectory();
}
