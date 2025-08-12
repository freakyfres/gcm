import { app } from "electron";
import { join } from "node:path";

export const DATA_DIR = process.env.GCM_USER_DATA_DIR ?? app.getPath("userData");
export const EXT_CACHE_DIR = join(DATA_DIR, "rendererExtensionCache");

export const REACT_DEVTOOLS_ID = "fmkadmapgofadopljbjfkapdkoienihi";
