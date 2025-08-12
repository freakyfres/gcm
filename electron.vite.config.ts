import tailwindcss from "@tailwindcss/vite";
import react from "@vitejs/plugin-react";

import { defineConfig } from "electron-vite";
import tsConfigPaths from "vite-tsconfig-paths";
export default defineConfig({
    renderer: {
        plugins: [
            react(),
            tailwindcss(),
            tsConfigPaths(),
        ],
        build: {
            outDir: "dist/renderer",
            sourcemap: "inline",
            rollupOptions: {
                output: {
                    format: "es",
                },
            },
        },
        css: {
            modules: {
                localsConvention: "camelCase",
            },
        },
    },
    preload: {
        build: {
            outDir: "dist/preload",
            sourcemap: "inline",
            rollupOptions: {
                output: {
                    format: "es",
                },
            },
        },
    },
    main: {
        build: {
            outDir: "dist/main",
            sourcemap: true,
            lib: {
                entry: "src/electron/main.ts",
            },
            rollupOptions: {
                output: {
                    format: "es",
                },
            },
        },
    },
});
