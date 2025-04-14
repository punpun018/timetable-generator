const { spawn } = require("child_process");

const runCpp3 = (jsonInput) => {
    return new Promise((resolve, reject) => {
        const cpp = spawn("./cpp-files/checker2.exe");

        let data = "";
        let error = "";

        cpp.stdout.on("data", (chunk) => {
            data += chunk.toString();
        });

        cpp.stderr.on("data", (chunk) => {
            error += chunk.toString();
        });

        cpp.on("close", (code) => {
            if (code !== 0 || error) {
                return reject(`CPP error: ${error || `Exited with code ${code}`}`);
            }

            try {
                const parsed = JSON.parse(data);
                resolve(parsed);
            } catch (e) {
                reject(`Invalid JSON from C++: ${e}`);
            }
        });

        // Send JSON input to C++
        cpp.stdin.write(JSON.stringify(jsonInput));
        cpp.stdin.end();
    });
};

module.exports = runCpp3;
