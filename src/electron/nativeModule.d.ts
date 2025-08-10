interface GoodClipboardManagerNative {
    registerClipboardWatcherCallback(cb: () => Promise<void>): void;
}
