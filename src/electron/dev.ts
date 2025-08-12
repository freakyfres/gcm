import { EXT_CACHE_DIR, REACT_DEVTOOLS_ID } from "./util/constants";
import { crxToZip } from "./util/crx2Zip";
import { exists, isDir } from "./util/fs";

import { session } from "electron";
import { unzip } from "fflate/node";
import { mkdir, readFile, readlink, rm, writeFile } from "node:fs/promises";
import { join } from "node:path";


export async function writeProcessId() {
    const rootDir = process.cwd();
    const launchFile = join(rootDir, ".vscode", "launch.json");
    const content = await readFile(launchFile, "utf-8");
    const processPath = await readlink("/proc/self/exe");

    const updatedContent = content
        .replace(/(?<="processId":\s*")\d*(?=",?\s*\/\/PID)/i, process.pid.toString())
        .replace(/(?<="program":\s*").*?(?=",?\s*\/\/PROGRAM)/i, processPath);


    await writeFile(launchFile, updatedContent);
}
async function extract(data: Buffer, outDir: string) {
    await mkdir(outDir, { recursive: true });
    return new Promise<void>((resolve, reject) => {
        unzip(data, (err, files) => {
            if (err)
                return void reject(err);
            Promise.all(Object.keys(files)
                .map(async (f) => {
                // Signature stuff
                // 'Cannot load extension with file or directory name
                // _metadata. Filenames starting with "_" are reserved for use by the system.';
                    if (f.startsWith("_metadata/"))
                        return;

                    if (f.endsWith("/"))
                        return void mkdir(join(outDir, f), { recursive: true });

                    const pathElements = f.split("/");
                    const name = pathElements.pop()!;
                    const directories = pathElements.join("/");
                    const dir = join(outDir, directories);

                    if (directories) {
                        await mkdir(dir, { recursive: true });
                    }

                    await writeFile(join(dir, name), files[f]);
                }))
                .then(() => resolve())
                .catch((err) => {
                    rm(outDir, {
                        recursive: true,
                        force: true,
                    });
                    reject(err);
                });
        });
    });
}

export async function installExtension(id: string) {
    if (await exists(EXT_CACHE_DIR)) {
        if (!await isDir(EXT_CACHE_DIR)) {
            throw new Error(`Extension cache dir ${EXT_CACHE_DIR} exists and is not a directory`);
        }
    } else {
        await mkdir(EXT_CACHE_DIR, { recursive: true });
    }

    const extPath = join(EXT_CACHE_DIR, id);

    if (await exists(extPath)) {
        if (!await isDir(extPath)) {
            throw new Error(`Extension path ${extPath} exists and is not a directory`);
        }
    } else {
        const extensionUrl = `https://clients2.google.com/service/update2/crx?response=redirect&acceptformat=crx2,crx3&x=id%3D${id}%26uc&prodversion=${process.versions.chrome}`;
        const res = Buffer.from(await (await fetch(extensionUrl)).arrayBuffer());

        await extract(crxToZip(res), extPath);
    }
    session.defaultSession.extensions.loadExtension(extPath);
}

export function installReactDevtools() {
    installExtension(REACT_DEVTOOLS_ID)
        .then(() => console.log("Installed React Devtools"))
        .catch((e) => console.error("Failed to install react devtools", e));
}
