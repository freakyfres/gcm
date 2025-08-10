import EventEmitter from "node:events";

export interface WindowStateEventMap {
    hide: [];
    show: [];
    close: [];
    closeImpl: [];
}

type K = keyof WindowStateEventMap;

type V<T extends K> = WindowStateEventMap[T];

class WindowState extends EventEmitter<WindowStateEventMap> {
    private _isHidden: boolean = false;

    get isHidden(): boolean {
        return this._isHidden;
    }

    // @ts-expect-error
    override emit<T extends K>(eventName: T, ...args: V<T>): boolean {
        // @ts-expect-error
        super.emit.call(this, eventName, ...args);
        if (eventName === "close") {
            this.emit("closeImpl");
        }
    }

    constructor() {
        super();
        this.on("hide", () => {
            this._isHidden = true;
        });
        this.on("show", () => {
            this._isHidden = false;
        });
    }
}

export const windowState = new WindowState();
