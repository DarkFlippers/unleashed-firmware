#!/usr/bin/env node
import fs from "node:fs";
import path from "node:path";
import { fileURLToPath } from "node:url";
import { SerialPort } from "serialport";
import prompts from "prompts";
import esbuild from "esbuild";
import json5 from "json5";

const __filename = fileURLToPath(import.meta.url);
const __dirname = path.dirname(__filename);

const cfwSdkName = "@darkflippers/fz-sdk-ul/";
const ofwSdkName = "@flipperdevices/fz-sdk/";

async function build(config) {
    await esbuild.build({
        entryPoints: ["./dist/index.js"],
        outfile: config.output,
        tsconfig: "./tsconfig.json",
        format: "cjs",
        bundle: true,
        minify: config.minify,
        external: [
            "@darkflippers/fz-sdk-ul/*"
        ],
        supported: {
            "array-spread": false,
            "arrow": false,
            "async-await": false,
            "async-generator": false,
            "bigint": false,
            "class": false,
            "const-and-let": true,
            "decorators": false,
            "default-argument": false,
            "destructuring": false,
            "dynamic-import": false,
            "exponent-operator": false,
            "export-star-as": false,
            "for-await": false,
            "for-of": false,
            "function-name-configurable": false,
            "function-or-class-property-access": false,
            "generator": false,
            "hashbang": false,
            "import-assertions": false,
            "import-meta": false,
            "inline-script": false,
            "logical-assignment": false,
            "nested-rest-binding": false,
            "new-target": false,
            "node-colon-prefix-import": false,
            "node-colon-prefix-require": false,
            "nullish-coalescing": false,
            "object-accessors": false,
            "object-extensions": false,
            "object-rest-spread": false,
            "optional-catch-binding": false,
            "optional-chain": false,
            "regexp-dot-all-flag": false,
            "regexp-lookbehind-assertions": false,
            "regexp-match-indices": false,
            "regexp-named-capture-groups": false,
            "regexp-set-notation": false,
            "regexp-sticky-and-unicode-flags": false,
            "regexp-unicode-property-escapes": false,
            "rest-argument": false,
            "template-literal": false,
            "top-level-await": false,
            "typeof-exotic-object-is-object": false,
            "unicode-escapes": false,
            "using": false,
        },
    });

    let outContents = fs.readFileSync(config.output, "utf8");
    outContents = "let exports = {};\n" + outContents;

    // Transform CFW SDK name to OFW SDK name so all firmwares understand it
    outContents = outContents.replaceAll(`require("${cfwSdkName}`, `require("${ofwSdkName}`);

    if (config.enforceSdkVersion) {
        const version = json5.parse(fs.readFileSync(path.join(__dirname, "package.json"), "utf8")).version;
        let [major, minor, _] = version.split(".");
        outContents = `checkSdkCompatibility(${major}, ${minor});\n${outContents}`;
    }

    fs.writeFileSync(config.output, outContents);
}

async function upload(config) {
    const appFile = fs.readFileSync(config.input, "utf8");
    const serialPorts = await SerialPort.list();

    let flippers = serialPorts
        .filter(x => x.serialNumber?.startsWith("flip_"))
        .map(x => ({ path: x.path, name: x.serialNumber.replace("flip_", "") }));

    if (!flippers.length) {
        // some Windows installations don't report the serial number correctly;
        // filter by STM VCP VID:PID instead
        flippers = serialPorts
            .filter(x => x?.vendorId === "0483" && x?.productId === "5740")
            .map(x => ({ path: x.path, name: x.path }));
    }

    if (!flippers.length) {
        console.error("No Flippers found");
        process.exit(1);
    }

    let portPath = flippers[0].path;
    if (flippers.length > 1) {
        port = (await prompts([{
            type: "select",
            name: "port",
            message: "Select Flipper to run the app on",
            choices: flippers.map(x => ({ title: x.serialNumber.replace("flip_", ""), value: x.path })),
        }])).port;
    }

    console.log(`Connecting to Flipper at ${portPath}`);
    let port = new SerialPort({ path: portPath, baudRate: 230400 });
    let received = "";
    let lastMatch = 0;
    async function waitFor(string, timeoutMs) {
        return new Promise((resolve, _reject) => {
            let timeout = undefined;
            if (timeoutMs) {
                timeout = setTimeout(() => {
                    console.error("Error: timeout");
                    process.exit(1);
                }, timeoutMs);
            }
            setInterval(() => {
                let idx = received.indexOf(string, lastMatch);
                if (idx !== -1) {
                    lastMatch = idx;
                    if (timeoutMs)
                        clearTimeout(timeout);
                    resolve();
                }
            }, 50);
        });
    }
    port.on("data", (data) => {
        received += data.toString();
    });

    await waitFor(">: ", 1000);
    console.log("Uploading application file");
    port.write(`storage remove ${config.output}\x0d`);
    port.drain();
    await waitFor(">: ", 1000);
    const appFileBuffer = Buffer.from(appFile, "utf8");
    port.write(`storage write_chunk ${config.output} ${appFileBuffer.length}\x0d`);
    await waitFor("Ready", 1000);
    port.write(appFileBuffer);
    await new Promise(resolve => port.drain(resolve));
    await waitFor(">: ", 1000);

    console.log("Launching application");
    port.write(`js ${config.output}\x0d`);
    port.drain();

    await waitFor("Running", 1000);
    process.stdout.write(received.slice(lastMatch));
    port.on("data", (data) => {
        process.stdout.write(data.toString());
    });
    process.on("exit", () => {
        port.write("\x03");
    });

    await waitFor("Script done!", 0);
    process.exit(0);
}

(async () => {
    const commands = {
        "build": build,
        "upload": upload,
    };

    const config = json5.parse(fs.readFileSync("./fz-sdk.config.json5", "utf8"));
    const command = process.argv[2];

    if (!Object.keys(commands).includes(command)) {
        console.error(`Unknown command ${command}. Supported: ${Object.keys(commands).join(", ")}`);
        process.exit(1);
    }

    await commands[command](config[command]);
})();
