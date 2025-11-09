const { execFile } = require("child_process");
const path = require("path");

function runPython2(jsonData) {
    console.log("python is running");
    return new Promise((resolve, reject) => {
        const pythonScript = path.join(__dirname, "../api/json_to_excel.py");

        // Execute Python script and pass JSON data through stdin
        const process = execFile("python", [pythonScript], {
            input: JSON.stringify(jsonData),  // Pass JSON as input to stdin
            encoding: "utf-8",
        });

        process.stdout.on("data", (data) => {
            console.log(`stdout: ${data}`);
        });

        process.stderr.on("data", (data) => {
            console.error(`stderr: ${data}`);
        });

        process.on("close", (code) => {
            if (code === 0) {
                resolve("Excel generated successfully.");
            } else {
                reject(`Python script failed with code ${code}`);
            }
        });
    });
}

module.exports = runPython2;
