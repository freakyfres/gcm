interface AppInfo {
    pid: number | null;
    /**
     * 0 if not available
     */
    iconData: Uint8Array;
    /**
     * empty if class not found
     */
    windowClass: string[];
    windowTitle: string;
    exePath: string | null;
}

interface ClipboardData {
    changeReason: "new_owner" | "destroy" | "close";
    changeTimestamp: number;
    appInfo: AppInfo | null;
}

declare var _: {
    registerClipboardWatcherCallback(cb: (data: ClipboardData) => Promise<void>): void;
};

export default _;
