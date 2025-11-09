const { spawn } = require('child_process');
const path = require('path');

const runCpp3 = (inputJson) => {
    return new Promise((resolve, reject) => {
        const exePath = path.join(__dirname, '../cpp-files/slot_suggester');
        const process = spawn(exePath);

        let output = '';
        process.stdout.on('data', (data) => {
            output += data.toString();
        });

        process.stderr.on('data', (data) => {
            console.error(`Error: ${data}`);
        });

        process.on('close', () => {
            try {
                resolve(JSON.parse(output));
            } catch (err) {
                reject('Failed to parse C++ output');
            }
        });

        process.stdin.write(JSON.stringify(inputJson));
        process.stdin.end();
    });
};

module.exports = runCpp3;
