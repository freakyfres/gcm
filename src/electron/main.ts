import { writeProcessId } from "./dev";
await writeProcessId();
import electronViteSvg from "./assets/electron-vite.svg?asset";
import { createSystemTray } from "./tray/systemTray";
import native, { ClipboardData, IconData } from "./gcmNative.node";
import { windowState } from "./WindowState";

import "./ipc";
import { createCanvas, ImageData } from "canvas";
import { app, BrowserWindow } from "electron";
import { writeFile } from "node:fs/promises";
import { join } from "node:path";

process.env.APP_ROOT = join(__dirname, "..");

// 🚧 Use ['ENV_NAME'] avoid vite:define plugin - Vite@2.x
export const { ELECTRON_RENDERER_URL } = process.env;
export const MAIN_DIST = join(process.env.APP_ROOT, "dist-electron");
export const RENDERER_DIST = join(process.env.APP_ROOT, "dist");
process.env.VITE_PUBLIC = ELECTRON_RENDERER_URL ? join(process.env.APP_ROOT, "public") : RENDERER_DIST;

function iconDataToPng(image: IconData): Buffer {
    const imageData = new ImageData(new Uint8ClampedArray(image.data), image.width, image.height);
    const canvas = createCanvas(image.width, image.height);
    const ctx = canvas.getContext("2d");

    ctx.putImageData(imageData, 0, 0);

    return canvas.toBuffer("image/png");
}
// eslint-disable-next-line @typescript-eslint/require-await
native.registerClipboardWatcherCallback(async (data: ClipboardData) => {
    if (data.appInfo) {
        const buf = iconDataToPng(data.appInfo.iconData);

        writeFile("image.png", buf);
    }
    console.log("Clipboard changed!");
});

let win: BrowserWindow;

function createWindow() {
    win = new BrowserWindow({
        icon: electronViteSvg,
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
