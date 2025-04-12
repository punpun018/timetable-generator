const { spawn } = require("child_process");

function runCppWithJson(inputJson) {
    // console.log("Passing to C++:", JSON.stringify(inputJson, null, 2));
    return new Promise((resolve, reject) => {
        const cpp = spawn("./cpp-files/generator.exe");
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

module.exports = runCppWithJson;
