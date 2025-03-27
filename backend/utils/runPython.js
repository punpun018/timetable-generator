const { spawn } = require("child_process");

const runPython = (fileBuffer) => {
    console.log(`Received file buffer of size: ${fileBuffer.length}`);

    return new Promise((resolve, reject) => {
        const pythonProcess = spawn("python", ["../timetable-generator/python-scripts/excel_Parser.py"]);

        pythonProcess.stdin.write(fileBuffer);
        pythonProcess.stdin.end();

        let data = "";

        pythonProcess.stdout.on("data", (chunk) => {
            data += chunk.toString();
        });

        pythonProcess.stderr.on("data", (error) => {
            console.error(`Error from Python script: ${error.toString()}`);
            reject(error.toString());
        });

        pythonProcess.on("close", () => {
            resolve(JSON.parse(data));
        });
    });
};

module.exports = { runPython };
