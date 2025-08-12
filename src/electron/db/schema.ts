import { int, sqliteTable } from "drizzle-orm/sqlite-core";

export const clipboardHistoryTable = sqliteTable("clipboard_history", {
    id: int()
        .primaryKey({ autoIncrement: true }),
});
