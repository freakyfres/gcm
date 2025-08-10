import { QuitAppItem } from "./items/QuitAppItem";
import { ShowHideWindowItem } from "./items/ShowHideWindowItem";
import { TrayItem } from "./items/TrayItem";

import { Menu, Tray } from "electron";
import { join } from "node:path";


class TrayMenu {
    private menu: Menu = new Menu();
    private items: TrayItem[] = [];

    constructor(private readonly tray: Electron.Tray) {
    }

    public setMenu() {
        this.tray.setContextMenu(this.menu);
    }

    private updateMenu() {
        const menu = new Menu();

        this.items.map((item) => item.makeMenuItem())
            .forEach((menuItem) => {
                menu.append(menuItem);
            });
        this.menu = menu;
        this.setMenu();
    }


    public addItem(item: TrayItem) {
        this.items.push(item);
        this.menu.append(item.makeMenuItem());
        item.on("invalidate", () => {
            this.updateMenu();
        });
    }
}

export function createSystemTray() {
    const tray = new Tray(join(process.env.VITE_PUBLIC || process.env.APP_ROOT, "icon.png"));
    const trayMenu = new TrayMenu(tray);

    trayMenu.addItem(new ShowHideWindowItem());
    trayMenu.addItem(new QuitAppItem());
    trayMenu.setMenu();
}
