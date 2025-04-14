const express = require("express");
const multer = require("multer");
const { runPython } = require("../utils/runPython");
const runCppWithJson = require("../utils/runCpp"); // <--- add this
const runCpp2WithJson = require("../utils/runCpp2"); // <--- add this
const router = express.Router();

// Configure Multer for file uploads
const storage = multer.memoryStorage();
const upload = multer({ storage });

router.post("/", upload.single("file"), async (req, res) => {
    console.log(`Received request with file: ${req.file ? req.file.originalname : 'No file'}`);

    if (!req.file) return res.status(400).json({ error: "No file uploaded" });

    try {
        const { days, slots, strength } = req.body;

        // Step 1: Run Python to get adjacencyGraph
        const adjacencyGraph = await runPython(req.file.buffer);
        adjacencyGraph.adjacencyGraph.numberOfDays = parseInt("10");
        adjacencyGraph.adjacencyGraph.numberOfSlots = parseInt("2");
        adjacencyGraph.adjacencyGraph.maxStrengthPerSlot = parseInt("1000");

        // Step 2: Pass full JSON to C++
        const finalTimetable = await runCppWithJson(adjacencyGraph);

        // Step 3: Pass full JSON to C++2
        const finaloTimetable = await runCpp2WithJson(finalTimetable);

        // Step 4: Send timetable to frontend (this includes the config)
        res.json(finaloTimetable);

    } catch (error) {
        console.error("Error:", error);
        if (!res.headersSent) {
            res.status(500).json({ error: "Error processing file", details: error.toString() });
        }
    }
});


module.exports = router;
