const { spawn } = require("child_process");
const path = require("path");

function runCheckerWithJson(inputJson) {
    return new Promise((resolve, reject) => {
        const exePath = path.join(__dirname, "..", "cpp-files", "checker2");
        const cpp = spawn(exePath);

        let output = "";
        let errorOutput = "";

        cpp.stdout.on("data", (data) => {
            output += data.toString();
        });

        cpp.stderr.on("data", (data) => {
            errorOutput += data.toString();
        });

        cpp.on("close", (code) => {
            if (code !== 0) {
                reject(`C++ exited with code ${code}: ${errorOutput}`);
            } else {
                try {
                    const result = JSON.parse(output);
                    if (result.error) {
                        reject(result.error);
                    } else {
                        resolve(result);
                    }
                } catch (err) {
                    reject("Failed to parse C++ output: " + output);
                }
            }
        });

        cpp.stdin.write(JSON.stringify(inputJson));
        cpp.stdin.end();
    });
}

module.exports = runCheckerWithJson;
