import { MenuItem } from "electron";
import EventEmitter from "node:events";

interface TrayItemEventMap {
    invalidate: [];
    click: [menuItem: MenuItem, browserWindow: (Electron.BaseWindow) | (undefined), event: Electron.KeyboardEvent];
}

export abstract class TrayItem extends EventEmitter<TrayItemEventMap> {
    private static nextId = 1;
    protected readonly id: string = `tray-item-${TrayItem.nextId++}`;

    abstract getLabel(): string | undefined;


    public makeMenuItem(): Electron.MenuItem {
        return new MenuItem({
            id: this.id,
            label: this.getLabel(),
            click: (menuItem, browserWindow, event) => {
                this.emit("click", menuItem, browserWindow, event);
            },
        });
    }
}
