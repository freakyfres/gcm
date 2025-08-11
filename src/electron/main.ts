import { writeProcessId } from "./dev";
await writeProcessId();
import { createSystemTray } from "./tray/systemTray";
import native from "./gcmNative.node";
import { windowState } from "./WindowState";

import "./ipc";
import { app, BrowserWindow } from "electron";
import { join } from "node:path";
import { inspect } from "node:util";

process.env.APP_ROOT = join(__dirname, "..");
console.log(native);

// 🚧 Use ['ENV_NAME'] avoid vite:define plugin - Vite@2.x
export const { ELECTRON_RENDERER_URL } = process.env;
export const MAIN_DIST = join(process.env.APP_ROOT, "dist-electron");
export const RENDERER_DIST = join(process.env.APP_ROOT, "dist");
process.env.VITE_PUBLIC = ELECTRON_RENDERER_URL ? join(process.env.APP_ROOT, "public") : RENDERER_DIST;

console.log("foobar");
// eslint-disable-next-line @typescript-eslint/require-await
native.registerClipboardWatcherCallback(async (data) => {
    console.log(inspect(data, { depth: 5 }));
    console.log("Clipboard changed!");
});

let win: BrowserWindow;


function createWindow() {
    win = new BrowserWindow({
        icon: join(process.env.VITE_PUBLIC, "electron-vite.svg"),
        webPreferences: {
            preload: join(__dirname, "preload.mjs"),
        },
        alwaysOnTop: true,
        autoHideMenuBar: true,
        transparent: true,
        frame: false,
    });

    if (ELECTRON_RENDERER_URL) {
        win.loadURL(ELECTRON_RENDERER_URL);
    } else {
        win.loadFile(join(RENDERER_DIST, "index.html"));
    }
}

windowState.on("show", () => {
    if (win) {
        win.show();
    }
});

windowState.on("hide", () => {
    if (win) {
        win.hide();
    }
});

windowState.on("closeImpl", () => {
    win.close();
    process.exit(0);
});

process.on("uncaughtException", (error) => {
    console.error("Uncaught Exception:", error);
});

app.whenReady()
    .then(createWindow)
    .then(createSystemTray);
