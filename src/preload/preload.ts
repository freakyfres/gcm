import { contextBridge, ipcRenderer } from "electron";

// --------- Expose some API to the Renderer process ---------
contextBridge.exposeInMainWorld("windowStateAPI", {
    show() {
        ipcRenderer.send("show");
    },
    hide() {
        ipcRenderer.send("hide");
    },
    close() {
        ipcRenderer.send("close");
    },
});
