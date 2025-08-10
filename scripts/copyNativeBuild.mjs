import { copyFile, mkdir } from "fs/promises";
import { join } from "path";

const __dirname = import.meta.dirname;

const NATIVE_NAME = "goodClipboardManagerNative.node";

const ROOT_DIR = join(__dirname, "..");
const NATIVE_DIR = join(ROOT_DIR, "native");
const BUILD_DIR = join(NATIVE_DIR, "build", "Debug");

const OUT_DIR = join(ROOT_DIR, "src", "electron");

await mkdir(OUT_DIR, { recursive: true });

await copyFile(join(BUILD_DIR, NATIVE_NAME), join(OUT_DIR, "gcmNative.node"));