import { TrayItem } from "./TrayItem";
import { windowState } from "../../WindowState";

export class ShowHideWindowItem extends TrayItem {
    constructor() {
        super();
        windowState.on("hide", () => this.setState("show"));
        windowState.on("show", () => this.setState("hide"));
        this.on("click", (menuItem, browserWindow, event) => {
            windowState.emit(this.state);
        });
    }

    private state: "show" | "hide" = windowState.isHidden ? "show" : "hide";

    private setState(newState: "show" | "hide") {
        if (this.state !== newState) {
            this.state = newState;
            this.emit("invalidate");
        }
    }

    getLabel(): string | undefined {
        return `${this.state.charAt(0)
            .toUpperCase() + this.state.slice(1)} Window`;
    }
}
