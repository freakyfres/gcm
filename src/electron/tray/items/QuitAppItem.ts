import { TrayItem } from "./TrayItem";
import { windowState } from "../../WindowState";

export class QuitAppItem extends TrayItem {
    constructor() {
        super();
        this.on("click", () => {
            windowState.emit("close");
        });
    }

    getLabel(): string | undefined {
        return "Quit App";
    }
}
