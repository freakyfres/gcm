import EventEmitter from "events";


interface ClipboardHistoryEventMap {
    change: [];
}

interface ClipboardItemBase {
    timestamp: number;
    copyingApplication: string | null;
    copyingApplicationThumbnail: Buffer | string | null;
}

interface ClipboardTextItem {
    type: "text";
    content: string;
}

interface ClipboardHTMLItem {
    type: "html";
    content: string;
}

interface ClipboardImageItem {
    type: "image";
    format: "png" | "jpeg" | "bmp";
    content: Buffer;
}

interface ClipboardRTFItem {
    type: "rtf";
    content: string;
}


type ClipboardItem
    = & (ClipboardTextItem | ClipboardHTMLItem | ClipboardImageItem | ClipboardRTFItem)
      & ClipboardItemBase;

class ClipboardHistory extends EventEmitter<ClipboardHistoryEventMap> {

}

export const clipboardHistory = new ClipboardHistory();
