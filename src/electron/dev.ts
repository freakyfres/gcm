import { readFile, readlink, writeFile } from "node:fs/promises";
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
