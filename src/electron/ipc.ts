import { windowState } from "./WindowState";

import { ipcMain } from "electron";

ipcMain.on("hide", () => {
    windowState.emit("hide");
});

ipcMain.on("show", () => {
    windowState.emit("show");
});

ipcMain.on("close", () => {
    windowState.emit("close");
});
