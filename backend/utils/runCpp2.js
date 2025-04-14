const { spawn } = require("child_process");

function runCheckerWithJson(inputJson) {
    return new Promise((resolve, reject) => {
        const cpp = spawn("./cpp-files/checker.exe");
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
