const express = require("express");
const multer = require("multer");
const { runPython } = require("../utils/runPython");
const runCppWithJson = require("../utils/runCpp");
const runCpp2WithJson = require("../utils/runCpp2");
const runCpp4WithJson = require("../utils/runCpp4");
const runCpp6WithJson = require("../utils/runCpp6");
// const runCpp5WithJson = require("../utils/runCpp5");
const router = express.Router();

const storage = multer.memoryStorage();
const upload = multer({ storage });

router.post("/", upload.single("file"), async (req, res) => {
    console.log(`Received request with file: ${req.file ? req.file.originalname : 'No file'}`);

    if (!req.file) return res.status(400).json({ error: "No file uploaded" });

    try {
        const { days, slots, strength } = req.body;

        // Step 1: Run Python to get adjacencyGraph
        const adjacencyGraph = await runPython(req.file.buffer);
        adjacencyGraph.adjacencyGraph.numberOfDays = parseInt(days);
        adjacencyGraph.adjacencyGraph.numberOfSlots = parseInt(slots);
        adjacencyGraph.adjacencyGraph.maxStrengthPerSlot = parseInt(strength);

        console.log(adjacencyGraph.adjacencyGraph.numberOfSlots);
        let finaloTimetable;

        if (adjacencyGraph.adjacencyGraph.numberOfSlots == 2) {
            const finalTimetable = await runCppWithJson(adjacencyGraph);
            finaloTimetable = await runCpp2WithJson(finalTimetable);
        } else if (adjacencyGraph.adjacencyGraph.numberOfSlots == 3) {
            // Example for slots != 2
            const finalTimetable = await runCpp4WithJson(adjacencyGraph);
            finaloTimetable = await runCpp2WithJson(finalTimetable);
        }
        else if (adjacencyGraph.adjacencyGraph.numberOfSlots == 4) {
            // Example for slots != 2
            const finalTimetable = await runCpp6WithJson(adjacencyGraph);
            finaloTimetable = await runCpp2WithJson(finalTimetable);
        }

        res.json(finaloTimetable);
    } catch (error) {
        console.error("Error:", error);
        if (!res.headersSent) {
            res.status(500).json({ error: "Error processing file", details: error.toString() });
        }
    }
});


module.exports = router;