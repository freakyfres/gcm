export interface IconData {
    width: number;
    height: number;
    data: Uint8Array;
}

export interface AppInfo {
    pid?: number;
    /**
     * empty if not available
     */
    iconData: IconData;
    /**
     * empty if class not found
     */
    windowClass: string[];
    windowTitle: string;
    exePath?: string;
}

export interface ClipboardData {
    changeReason: "new_owner" | "destroy" | "close";
    changeTimestamp: number;
    appInfo?: AppInfo;
}

declare var _: {
    registerClipboardWatcherCallback(cb: (data: ClipboardData) => Promise<void>): void;
};

export default _;
