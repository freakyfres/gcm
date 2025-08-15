import { copyFile, mkdir } from "fs/promises";
import { join } from "path";

const __dirname = import.meta.dirname;

const NATIVE_NAME = "libgcm_native.so";

const ROOT_DIR = join(__dirname, "..");
const NATIVE_DIR = join(ROOT_DIR, "native");
const BUILD_DIR = join(NATIVE_DIR, "target", "debug");

const OUT_DIR = join(ROOT_DIR, "src", "electron");

await mkdir(OUT_DIR, { recursive: true });

await copyFile(join(BUILD_DIR, NATIVE_NAME), join(OUT_DIR, "gcmNative.node"));